#include <string>
#include <vector>
#include <random>
#include <cmath>
#include <unordered_set>
#include <algorithm>
#include <mutex>
#include <functional>
#include <cstring>
#include <memory>

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

Index::Node::Node(int id, std::string name, std::vector<double> descriptor, int layersCount, int neighboursCount, int neighboursCount0) :
id(id), name(std::move(name)), descriptor(std::move(descriptor)), maxLayer(layersCount - 1), layers(layersCount), layersMutexes(layersCount) {
	if (layersCount > 0) {
		layers[0].reserve(neighboursCount0);

		for (int i = 1; i < layersCount; ++i) {
			layers[i].reserve(neighboursCount);
		}
	}
}

Index::NodeList Index::Node::getNeighbourhood(int layer) {
	// TODO: check layer

	std::unique_lock<std::mutex> lock(layersMutexes[layer]);

	return layers[layer];
}

void Index::Node::setNeighbourhood(NodeList neighbourhood, int layer) {
	// TODO: check layer

	std::unique_lock<std::mutex> lock(layersMutexes[layer]);

	layers[layer] = std::move(neighbourhood);
}

void Index::Node::addNeighbour(const NodePtr &neighbour, int layer) {
	// TODO: check layer

	std::unique_lock<std::mutex> lock(layersMutexes[layer]);

	layers[layer].push_back(neighbour);
}

int Index::Node::getNeighbourhoodSize(int layer) {
	std::unique_lock<std::mutex> lock(layersMutexes[layer]);

	return layers[layer].size();
}

void Index::NodeQueue::push(NodeDistance item) {
	container.push_back(item);
	push_heap(container.begin(), container.end(), DistanceComparator());
}

void Index::NodeQueue::emplace(double distance, const NodePtr &node) {
	container.emplace_back(distance, node);
	push_heap(container.begin(), container.end(), DistanceComparator());
}

void Index::NodeQueue::pop() {
	pop_heap(container.begin(), container.end(), DistanceComparator());
	container.pop_back();
}

Index::Index(int descriptorSize, Settings settings) {
	this->descriptorSize = descriptorSize;
	this->metric = settings.metric;
	this->M = settings.M;
	this->M0 = settings.M0;
	this->efConstruction = std::max(settings.efConstruction, M);
	this->efSearch = settings.efSearch;
	this->mL = settings.mL;
};

void Index::copy(Index &&other) {
	entryPoint = other.entryPoint;
	descriptorSize = other.descriptorSize;
	metric = other.metric;
	M = other.M;
	M0 = other.M0;
	efConstruction = other.efConstruction;
	efSearch = other.efSearch;
	mL = other.mL;
}

Index::NodePtr Index::getEntryPoint() {
	std::unique_lock<std::mutex> lock(entryMutex);

	return entryPoint;
}

void Index::setEntryPoint(const NodePtr &newEntryPoint) {
	std::unique_lock<std::mutex> lock(entryMutex);

	if (entryPoint && entryPoint->maxLayer >= newEntryPoint->maxLayer) {
		return;
	}

	entryPoint = newEntryPoint;
}

int Index::getSize() {
	std::unique_lock<std::mutex> lock(idMutex);

	return maxId + 1;
}

int Index::generateId() {
	std::unique_lock<std::mutex> lock(idMutex);

	return ++maxId;
}

Index::NodePtr Index::createNode(std::string name, std::vector<double> descriptor, int layer) {
	return std::make_shared<Node>(generateId(), std::move(name), std::move(descriptor), layer + 1, M + 1, M0 + 1);
}

double Index::distance(const NodePtr &a, const NodePtr &b) {
	return metric->distance(a->descriptor, b->descriptor);
}

void Index::searchAtLayer(
	const NodePtr &target, const NodePtr &entry, int searchCount, int layer,
	NodeQueue &candidates, bool *visited, int candidatesCount, NodeQueue &result
) {
	double entryDistance = distance(target, entry);
	result.emplace(entryDistance, entry);
	candidates.emplace(entryDistance, entry);
	visited[entry->id] = true;

	while (!candidates.empty()) {
		NodeDistance candidate = candidates.nearest();

		if (candidate.distance > result.furthest().distance) {
			break;
		}

		NodeList neighbourhood = candidate.node->getNeighbourhood(layer);
		candidates.pop();

		for (NodePtr neighbour : neighbourhood) {
			if (neighbour->id >= candidatesCount || visited[neighbour->id]) {
				continue;
			}

			visited[neighbour->id] = true;
			double neighbourDistance = distance(target, neighbour);

			if (neighbourDistance < result.furthest().distance || result.size() < searchCount) {
				candidates.emplace(neighbourDistance, neighbour);
				result.emplace(neighbourDistance, neighbour);

				if (result.size() > searchCount) {
					result.pop();
				}
			}
		}
	}
}

void Index::selectNeighbours(
	const NodePtr &target, int count, int layer,
	NodeQueue &candidates, NodeList &result
) {
	while (candidates.size() > 0 && result.size() < count) {
		NodeDistance candidate = candidates.nearest();
		candidates.pop();

		bool isCloser = true;

		for (NodePtr resultNode : result) {
			if (distance(resultNode, candidate.node) < candidate.distance) {
				isCloser = false;
				break;
			}
		}

		if (isCloser) {
			result.push_back(candidate.node);
		}
	}
}

void Index::insert(std::string name, std::vector<double> descriptor) {
	// TODO: check descriptor size

	int nodeLayer = static_cast<int>(-std::log(Index::generateRand()) * mL);
	NodePtr newNode = createNode(std::move(name), std::move(descriptor), nodeLayer);

	NodePtr entry = getEntryPoint();

	if (!entry) {
		setEntryPoint(newNode);
		return;
	}

	int candidatesCount = getSize();

	NodeQueue candidates;
	candidates.reserve(candidatesCount);

	bool *visited = new bool[candidatesCount];
	memset(visited, false, candidatesCount);

	NodeQueue nearestNodes;
	nearestNodes.reserve(efConstruction + 1);

	int maxLayer = entry->maxLayer;

	for (int layer = maxLayer; layer > nodeLayer; --layer) {
		searchAtLayer(newNode, entry, 1, layer, candidates, visited, candidatesCount, nearestNodes);
		entry = nearestNodes.nearest().node;

		candidates.clear();
		memset(visited, false, candidatesCount);
		nearestNodes.clear();
	}

	NodeList neighbours;
	neighbours.reserve(M0 + 1);

	NodeList newNeighbours;
	NodeQueue sortedNeighbours;

	for (int layer = std::min(nodeLayer, maxLayer); layer >= 0; --layer) {
		searchAtLayer(newNode, entry, efConstruction, layer, candidates, visited, candidatesCount, nearestNodes);
		entry = nearestNodes.nearest().node;

		selectNeighbours(newNode, M, layer, nearestNodes, neighbours);

		for (NodePtr neighbour : neighbours) {
			newNode->addNeighbour(neighbour, layer);
			neighbour->addNeighbour(newNode, layer);
		}

		int maxM = (layer == 0) ? M0 : M;

		for (NodePtr neighbour : neighbours) {
			if (neighbour->getNeighbourhoodSize(layer) > maxM) {
				NodeList neighbourhood = neighbour->getNeighbourhood(layer);

				if (newNeighbours.capacity() == 0) {
					newNeighbours.reserve(M0 + 1);
					sortedNeighbours.reserve(M0 + 1);
				}

				for (NodePtr node : neighbourhood) {
					sortedNeighbours.emplace(distance(neighbour, node), node);
				}

				selectNeighbours(neighbour, maxM, layer, sortedNeighbours, newNeighbours);
				neighbour->setNeighbourhood(newNeighbours, layer);

				newNeighbours.clear();
				sortedNeighbours.clear();
			}
		}

		candidates.clear();
		memset(visited, false, candidatesCount);
		nearestNodes.clear();
		neighbours.clear();
	}

	if (nodeLayer > maxLayer) {
		setEntryPoint(newNode);
	}
}

std::vector<SearchResult> Index::search(std::vector<double> descriptor, int k) {
	if (!entryPoint) {
		return std::vector<SearchResult>();
	}

	// TODO: check descriptor size

	NodePtr node = std::make_shared<Node>(std::move(descriptor));

	int searchCount = std::max(efSearch, k);
	int candidatesCount = getSize();

	NodeQueue nearestNodes;
	nearestNodes.reserve(searchCount);

	NodeQueue candidates;
	candidates.reserve(candidatesCount);

	bool *visited = new bool[candidatesCount];
	memset(visited, false, candidatesCount);

	NodePtr entry = getEntryPoint();
	int maxLayer = entry->maxLayer;

	for (int layer = maxLayer; layer > 0; --layer) {
		searchAtLayer(node, entry, 1, layer, candidates, visited, candidatesCount, nearestNodes);
		entry = nearestNodes.nearest().node;

		candidates.clear();
		memset(visited, false, candidatesCount);
		nearestNodes.clear();
	}

	searchAtLayer(node, entry, std::max(efSearch, k), 0, candidates, visited, candidatesCount, nearestNodes);

	int resultSize = std::min(k, nearestNodes.size());
	std::vector<SearchResult> result;
	result.reserve(resultSize);

	for (int i = 0; i < resultSize; ++i) {
		NodeDistance closeNode = nearestNodes[i];
		result.emplace_back(closeNode.node->name, closeNode.node->descriptor, closeNode.distance);
	}

	return result;
}

#include <iostream>
#include <queue>

void Index::save() {
	NodeList nodes;
	nodes.reserve(getSize());

	std::queue<NodePtr> candidates;
	candidates.push(entryPoint);

	std::vector<bool> visited(10000, false);
	visited[entryPoint->id] = true;

	int counter = 0;

	while (!candidates.empty()) {
		counter++;

		NodePtr candidate = candidates.front();
		candidates.pop();

		nodes.push_back(candidate);

		NodeList &neighbours = candidate->layers[0];

		for (NodePtr neighbour : neighbours) {
			if (!visited[neighbour->id]) {
				candidates.push(neighbour);
				visited[neighbour->id] = true;
			}
		}
	}

	std::vector<int> ids;

	for (int i = 1; i < 10000 && i < visited.size(); ++i) {
		if (!visited[i]) {
			ids.push_back(i);
		}
	}

	std::cout << "Lost: " << ids.size() << std::endl;
}
