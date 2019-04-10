#ifndef INDEX_H
#define INDEX_H

#include <string>
#include <vector>
#include <unordered_set>
#include <random>
#include <cmath>
#include <mutex>

class Metric {
public:
	virtual double distance(const std::vector<double> &a, const std::vector<double> &b) = 0;
};

class Euclidean : public Metric {
public:
	double distance(const std::vector<double> &a, const std::vector<double> &b) override;
};

struct Settings {
	Metric *metric = new Euclidean();
	int M = 16;
	int M0 = 2 * M;
	int efConstruction = 100;
	int efSearch = 10;
	double mL = 1.0 / std::log(M);

	void tuneToM() {
		M0 = 2 * M;
		mL = 1.0 / std::log(M);
	}
};

struct SearchResult {
	std::string name;
	std::vector<double> descriptor;
	double distance;

	SearchResult(std::string name, std::vector<double> descriptor, double distance) :
		name(std::move(name)), descriptor(std::move(descriptor)), distance(distance) {}
};

class Index {
	class Node;
	struct NodeDistance;
	class NodeQueue;

	using NodeList = std::vector<Node*>;

	static std::mt19937 gen;
	static std::uniform_real_distribution<double> dist;

	Node *entryPoint = nullptr;
	int size = 0;

	std::mutex entryMutex;
	std::mutex sizeMutex;

	int descriptorSize;

	Metric *metric;
	int M;
	int M0;
	int efConstruction;
	int efSearch;
	double mL;
	bool extendCandidates;
	bool keepPrunedConnections;

	static double generateRand() {
		return dist(gen);
	}

	void copy(Index &&other);

	double distance(Node *a, Node *b);

	Node* getEntryPoint();
	void setEntryPoint(Node *newEntryPoint);

	int getSize();
	void increaseSize();

	Node* createNode(std::string name, std::vector<double> descriptor, int layer);
	Node* createNode(std::vector<double> descriptor);

	void searchAtLayer(Node *target, Node *entry, int count, int layer,
		NodeQueue &candidates, std::unordered_set<Node*> &visited, NodeQueue &result);

	void selectNeighbours(Node *target, int count, int layer,
		NodeQueue &candidates, NodeList &result);

public:
	Index(int descriptorSize, Settings settings = Settings());

	Index(const Index&) = delete;
	Index& operator=(Index&) = delete;

	Index(Index &&other) {
		copy(std::move(other));
	}

	Index& operator=(Index &&other) {
		copy(std::move(other));
	}

	int getDescriptorSize() {
		return descriptorSize;
	}

	void insert(std::string name, std::vector<double> descriptor);
	std::vector<SearchResult> search(std::vector<double> descriptor, int k);
};

class Index::Node {
	std::vector<NodeList> layers;
	int maxLayer = -1;

	std::mutex mutex;

public:
	std::string name;
	std::vector<double> descriptor;

	Node(std::string name, std::vector<double> descriptor, int layersCount, int neighboursCount, int neighboursCount0);

	int getMaxLayer() {
		return maxLayer;
	}

	NodeList getNeighbourhood(int layer);
	void setNeighbourhood(NodeList neighbourhood, int layer);
	void addNeighbour(Node *neighbour, int layer);
	int getNeighbourhoodSize(int layer);
};

struct Index::NodeDistance {
	double distance;
	Node *node;

	NodeDistance(double distance, Node *node) : distance(distance), node(node) {}
};

class Index::NodeQueue {
	class DistanceComparator {
	public:
		bool operator()(const NodeDistance &a, const NodeDistance &b) {
			return a.distance > b.distance;
		}
	};

	std::vector<NodeDistance> container;

public:
	void push(NodeDistance item);
	void emplace(double distance, Node *node);
	void pop();

	const NodeDistance& nearest() {
		return container.front();
	}

	const NodeDistance& furthest() {
		return container.back();
	}

	int size() {
		return static_cast<int>(container.size());
	}

	int capacity() {
		return container.capacity();
	}

	bool empty() {
		return container.empty();
	}

	const NodeDistance& operator[](int index) {
		return container[index];
	}

	std::vector<NodeDistance>::const_iterator begin() const {
		return container.cbegin();
	}

	std::vector<NodeDistance>::const_iterator end() const {
		return container.cend();
	}

	void reserve(int size) {
		container.reserve(size);
	}

	void clear() {
		container.clear();
	}
};

#endif