#ifndef INDEX_H
#define INDEX_H

#include <string>
#include <vector>
#include <random>
#include <cmath>

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
	bool extendCandidates = false;
	bool keepPrunedConnections = false;

	void tuneToM() {
		M0 = 2 * M;
		mL = 1.0 / std::log(M);
	}
};

struct SearchResult {
	std::string name;
	std::vector<double> descriptor;
	double distance;
};

class Index {
	class Node;
	struct NodeDistance;
	class NodeQueue;

	using NodeList = std::vector<Node*>;

	static std::mt19937 gen;
	static std::uniform_real_distribution<double> dist;

	Node *entryPoint = nullptr;

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

	double distance(Node *a, Node *b);

	NodeQueue searchAtLayer(Node *target, Node *entry, int count, int layer);
	NodeList selectNeighbours(Node *target, NodeQueue candidates, int count, int layer);

public:
	Index(int descriptorSize, Settings settings = Settings()) {
		this->metric = settings.metric;
		this->M = settings.M;
		this->M0 = settings.M0;
		this->efConstruction = std::max(settings.efConstruction, M);
		this->efSearch = settings.efSearch;
		this->mL = settings.mL;
		this->extendCandidates = settings.extendCandidates;
		this->keepPrunedConnections = settings.keepPrunedConnections;
	};

	void insert(std::string name, std::vector<double> descriptor);
	std::vector<SearchResult> search(std::vector<double> descriptor, int k);
};

class Index::Node {
	std::vector<NodeList> layers;
	int maxLayer;

public:
	std::string name;
	std::vector<double> descriptor;

	Node(std::string name, std::vector<double> descriptor, int maxlayer) :
		name(std::move(name)), descriptor(std::move(descriptor)), maxLayer(maxLayer), layers(maxLayer + 1) {}

	Node(std::vector<double> descriptor) : Node(std::string(), std::move(descriptor), -1) {}

	int getMaxLayer() {
		return maxLayer;
	}

	const NodeList& getNeighbourhood(int layer);
	void setNeighbourhood(NodeList neighbourhood, int layer);
	void addNeighbour(Node *neighbour, int layer);
};

struct Index::NodeDistance {
	double distance;
	Node *node;
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
};

#endif