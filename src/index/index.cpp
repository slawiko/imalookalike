#include <string>
#include <vector>
#include <random>
#include <cmath>
#include <unordered_set>
#include <algorithm>
#include <mutex>
#include <functional>

#include "index.h"

std::random_device rd;
std::mt19937 Index::gen(rd());
std::uniform_real_distribution<double> Index::dist(0.0, 1.0);

double Euclidean::distance(const std::vector<double> &a, const std::vector<double> &b) {
	// TODO: check descriptor sizes equality

	double ac = 0.0;

	for (int i = 0; i < a.size(); ++i) {
		ac += std::pow(a[i] - b[i], 2);
	}

	return std::sqrt(ac);
}

Index::NodeList Index::Node::getNeighbourhood(int layer) {
	// TODO: check layer

	std::unique_lock<std::mutex> lock(mutex);

	return layers[layer];
}

void Index::Node::setNeighbourhood(NodeList neighbourhood, int layer) {
	// TODO: check layer

	std::unique_lock<std::mutex> lock(mutex);

	layers[layer] = std::move(neighbourhood);
}

void Index::Node::addNeighbour(Node *neighbour, int layer) {
	// TODO: check layer

	std::unique_lock<std::mutex> lock(mutex);

	layers[layer].push_back(neighbour);
}

void Index::NodeQueue::push(NodeDistance item) {
	container.push_back(item);
	push_heap(container.begin(), container.end(), DistanceComparator());
}

void Index::NodeQueue::pop() {
	pop_heap(container.begin(), container.end(), DistanceComparator());
	container.pop_back();
}

double Index::distance(Node *a, Node *b) {
	return metric->distance(a->descriptor, b->descriptor);
}

Index::NodeQueue Index::searchAtLayer(Node *target, Node *entry, int count, int layer) {
	NodeQueue candidates;
	NodeQueue result;
	std::unordered_set<Node*> visited;

	double entryDistance = distance(target, entry);
	result.push(NodeDistance{ entryDistance, entry });
	candidates.push(NodeDistance{ entryDistance, entry });
	visited.insert(entry);

	while (!candidates.empty()) {
		NodeDistance candidate = candidates.nearest();

		if (candidate.distance > result.furthest().distance) {
			break;
		}

		const NodeList &neighbourhood = candidate.node->getNeighbourhood(layer);
		candidates.pop();

		for (Node *neighbour : neighbourhood) {
			if (visited.find(neighbour) != visited.end()) {
				continue;
			}

			visited.insert(neighbour);
			double neighbourDistance = distance(target, neighbour);

			if (neighbourDistance < result.furthest().distance || result.size() < count) {
				candidates.push(NodeDistance{ neighbourDistance, neighbour });
				result.push(NodeDistance{ neighbourDistance, neighbour });

				if (result.size() > count) {
					result.pop();
				}
			}
		}
	}

	return result;
}

Index::NodeList Index::selectNeighbours(Index::Node *target, NodeQueue candidates, int count, int layer) {
	NodeQueue discarded;
	NodeList result;

	if (extendCandidates) {
		NodeQueue origin(candidates);
		std::unordered_set<Node*> visited;

		for (const NodeDistance &originNode : origin) {
			visited.insert(originNode.node);
		}

		for (const NodeDistance &originNode : origin) {
			const NodeList &neighbourhood = originNode.node->getNeighbourhood(layer);

			for (Node *neighbour : neighbourhood) {
				if (visited.find(neighbour) != visited.end()) {
					continue;
				}

				candidates.push(NodeDistance{ distance(target, neighbour), neighbour });
				visited.insert(neighbour);
			}
		}
	}

	while (candidates.size() > 0 && result.size() < count) {
		NodeDistance candidate = candidates.nearest();
		candidates.pop();

		bool isCloser = true;

		for (Node *resultNode : result) {
			if (distance(resultNode, candidate.node) < candidate.distance) {
				isCloser = false;
				break;
			}
		}

		if (isCloser) {
			result.push_back(candidate.node);
		}
		else {
			discarded.push(candidate);
		}
	}

	if (keepPrunedConnections) {
		while (discarded.size() > 0 && result.size() < count) {
			result.push_back(discarded.nearest().node);
			discarded.pop();
		}
	}

	return result;
}

void Index::insert(std::string name, std::vector<double> descriptor) {
	// TODO: check descriptor size

	int nodeLayer = static_cast<int>(-std::log(Index::generateRand()) * mL);
	Node *newNode = new Node(std::move(name), std::move(descriptor), nodeLayer);

	if (!entryPoint) {
		entryPoint = newNode;
		return;
	}

	Node *entry = entryPoint;
	int maxLayer = entry->getMaxLayer();

	for (int layer = maxLayer; layer > nodeLayer; --layer) {
		NodeQueue nearestNodes = searchAtLayer(newNode, entry, 1, layer);
		entry = nearestNodes.nearest().node;
	}

	for (int layer = std::min(nodeLayer, maxLayer); layer >= 0; --layer) {
		NodeQueue nearestNodes = searchAtLayer(newNode, entry, efConstruction, layer);
		entry = nearestNodes.nearest().node;

		threadPool.enqueu([this](Node *newNode, NodeQueue &nearestNodes, int layer) mutable {
			NodeList neighbours = selectNeighbours(newNode, std::move(nearestNodes), M, layer);

			for (Node *neighbour : neighbours) {
				newNode->addNeighbour(neighbour, layer);
				neighbour->addNeighbour(newNode, layer);
			}

			int maxM = (layer == 0) ? M0 : M;

			for (Node *neighbour : neighbours) {
				const NodeList &neighbourhood = neighbour->getNeighbourhood(layer);

				if (neighbourhood.size() > maxM) {
					NodeQueue sortedNeighbourhood;

					for (Node *node : neighbourhood) {
						sortedNeighbourhood.push(NodeDistance{ distance(neighbour, node), node });
					}

					neighbour->setNeighbourhood(selectNeighbours(neighbour, std::move(sortedNeighbourhood), maxM, layer), layer);
				}
			}
		}, newNode, std::move(nearestNodes), layer);
	}

	threadPool.wait();

	if (nodeLayer > maxLayer) {
		entryPoint = newNode;
	}
}

std::vector<SearchResult> Index::search(std::vector<double> descriptor, int k) {
	if (!entryPoint) {
		return std::vector<SearchResult>();
	}

	// TODO: check descriptor size

	Node *node = new Node(std::move(descriptor));

	Node *entry = entryPoint;
	int maxLayer = entry->getMaxLayer();

	for (int layer = maxLayer; layer > 0; --layer) {
		NodeQueue nearestNodes = searchAtLayer(node, entry, 1, layer);
		entry = nearestNodes.nearest().node;
	}

	NodeQueue nearestNodes = searchAtLayer(node, entry, std::max(efSearch, k), 0);

	std::vector<SearchResult> result;
	int resultSize = std::min(k, nearestNodes.size());

	for (int i = 0; i < resultSize; ++i) {
		NodeDistance closeNode = nearestNodes[i];
		result.push_back(SearchResult{ closeNode.node->name, closeNode.node->descriptor, closeNode.distance });
	}

	return result;
}
