
#ifndef POWERPATH_H
#define POWERPATH_H

#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <functional>
#include <stdexcept>

using namespace std;

// ==========================================
// 1. ELECTRICAL LOAD MODEL
// ==========================================

struct Load {
    int id;
    string name;
    string room;
    int power;
    int priority;
    bool active = false;
};

// ==========================================
// 2. CUSTOM PRIORITY QUEUE - MIN HEAP
// ==========================================

class LoadHeap {

private:
    vector<Load> heap;

    bool higherPriority(const Load& a, const Load& b) {
        if (a.priority != b.priority)
            return a.priority < b.priority;

        return a.id < b.id;
    }

    void heapifyUp(int index) {

        while (index > 0) {

            int parent = (index - 1) / 2;

            if (higherPriority(heap[parent], heap[index]))
                break;

            swap(heap[parent], heap[index]);

            index = parent;
        }
    }

    void heapifyDown(int index) {

        int n = heap.size();

        while (true) {

            int smallest = index;

            int left = 2 * index + 1;
            int right = 2 * index + 2;

            if (left < n &&
                higherPriority(heap[left], heap[smallest]))
                smallest = left;

            if (right < n &&
                higherPriority(heap[right], heap[smallest]))
                smallest = right;

            if (smallest == index)
                break;

            swap(heap[index], heap[smallest]);

            index = smallest;
        }
    }

public:

    void push(Load load) {

        heap.push_back(load);

        heapifyUp(heap.size() - 1);
    }

    Load pop() {

        if (heap.empty())
            throw runtime_error("Priority Queue is empty");

        Load result = heap[0];

        heap[0] = heap.back();

        heap.pop_back();

        if (!heap.empty())
            heapifyDown(0);

        return result;
    }

    bool empty() const {
        return heap.empty();
    }

    int size() const {
        return static_cast<int>(heap.size());
    }
};

// ==========================================
// 3. CUSTOM QUEUE - LINKED LIST
// ==========================================

template <typename T>
class RestorationQueue {

private:

    struct Node {
        T data;
        Node* next;

        Node(T value) : data(value), next(nullptr) {}
    };

    Node* frontNode = nullptr;
    Node* rearNode = nullptr;

public:

    ~RestorationQueue() {
        while (!empty())
            pop();
    }

    void push(T value) {

        Node* newNode = new Node(value);

        if (rearNode == nullptr) {

            frontNode = rearNode = newNode;

        } else {

            rearNode->next = newNode;
            rearNode = newNode;
        }
    }

    T pop() {

        if (empty())
            throw runtime_error("Queue is empty");

        Node* temp = frontNode;

        T value = temp->data;

        frontNode = frontNode->next;

        if (frontNode == nullptr)
            rearNode = nullptr;

        delete temp;

        return value;
    }

    bool empty() const {
        return frontNode == nullptr;
    }

    vector<T> snapshot() const {

        vector<T> result;

        Node* current = frontNode;

        while (current != nullptr) {

            result.push_back(current->data);

            current = current->next;
        }

        return result;
    }
};

// ==========================================
// 4. HOSTEL ELECTRICAL GRAPH
// ==========================================

class HostelGraph {

private:

    unordered_map<string, vector<string>> adj;

public:

    void addConnection(string a, string b) {

        adj[a].push_back(b);
        adj[b].push_back(a);
    }

    vector<string> BFS(string start) {

        vector<string> result;

        if (!adj.count(start))
            return result;

        unordered_map<string, bool> visited;

        queue<string> q;

        visited[start] = true;

        q.push(start);

        while (!q.empty()) {

            string current = q.front();

            q.pop();

            result.push_back(current);

            for (auto& next : adj[current]) {

                if (!visited[next]) {

                    visited[next] = true;

                    q.push(next);
                }
            }
        }

        return result;
    }

    vector<string> DFS(string start) {

        vector<string> result;

        if (!adj.count(start))
            return result;

        unordered_map<string, bool> visited;

        function<void(string)> dfs = [&](string node) {

            visited[node] = true;

            result.push_back(node);

            for (auto& next : adj[node]) {

                if (!visited[next])
                    dfs(next);
            }
        };

        dfs(start);

        return result;
    }
};

// ==========================================
// 5. HOSTEL TREE
// ==========================================

struct TreeNode {

    string name;

    vector<TreeNode*> children;

    TreeNode(string value) {
        name = value;
    }

    ~TreeNode() {

        for (auto child : children)
            delete child;
    }
};

class HostelTree {

public:

    TreeNode* root;

    HostelTree() {
        root = new TreeNode("HOSTEL");
    }

    ~HostelTree() {
        delete root;
    }

    void addFloor(string floor) {

        root->children.push_back(new TreeNode(floor));
    }

    void addRoom(string floor, string room) {

        for (auto f : root->children) {

            if (f->name == floor) {

                f->children.push_back(new TreeNode(room));

                return;
            }
        }

        addFloor(floor);

        addRoom(floor, room);
    }

    void display(TreeNode* node, int level = 0) {

        cout << string(level * 2, ' ')
             << node->name << endl;

        for (auto child : node->children)
            display(child, level + 1);
    }
};

// ==========================================
// 6. ROOM MANAGEMENT - HASH MAP
// ==========================================

struct Room {

    string id;

    int floor;

    int consumption;
};

class RoomManager {

private:

    unordered_map<string, Room> rooms;

public:

    void addRoom(Room room) {

        rooms[room.id] = room;
    }

    bool exists(string id) const {

        return rooms.find(id) != rooms.end();
    }

    Room* search(string id) {

        auto it = rooms.find(id);

        if (it == rooms.end())
            return nullptr;

        return &it->second;
    }

    vector<Room> getAll() const {

        vector<Room> result;

        for (const auto& entry : rooms)
            result.push_back(entry.second);

        return result;
    }
};

// ==========================================
// 7. MERGE SORT
// ==========================================

void mergeSort(vector<Room>& arr, int left, int right) {

    if (left >= right)
        return;

    int mid = left + (right - left) / 2;

    mergeSort(arr, left, mid);

    mergeSort(arr, mid + 1, right);

    vector<Room> temp;

    int i = left;

    int j = mid + 1;

    while (i <= mid && j <= right) {

        if (arr[i].consumption <= arr[j].consumption)
            temp.push_back(arr[i++]);

        else
            temp.push_back(arr[j++]);
    }

    while (i <= mid)
        temp.push_back(arr[i++]);

    while (j <= right)
        temp.push_back(arr[j++]);

    for (int k = 0; k < temp.size(); k++)
        arr[left + k] = temp[k];
}

// ==========================================
// 8. BINARY SEARCH
// ==========================================

int binarySearch(const vector<Room>& rooms, string id) {

    int low = 0;

    int high = static_cast<int>(rooms.size()) - 1;

    while (low <= high) {

        int mid = low + (high - low) / 2;

        if (rooms[mid].id == id)
            return mid;

        if (rooms[mid].id < id)
            low = mid + 1;

        else
            high = mid - 1;
    }

    return -1;
}

// ==========================================
// 9. GREEDY POWER ALLOCATION
// ==========================================

struct AllocationResult {

    vector<Load> allocated;

    vector<Load> waiting;

    int totalPower = 0;

    int remainingPower = 0;
};

AllocationResult allocatePower(
    vector<Load> loads,
    int availablePower
) {

    if (availablePower < 0)
        throw invalid_argument("Invalid power supply");

    int criticalRequired = 0;

    for (const auto& load : loads) {

        if (load.power <= 0 ||
            load.priority < 1 ||
            load.priority > 4)
            throw invalid_argument("Invalid load configuration");

        if (load.priority == 1)
            criticalRequired += load.power;
    }

    if (criticalRequired > availablePower)
        throw runtime_error(
            "Insufficient power for critical loads!"
        );

    LoadHeap heap;

    for (const auto& load : loads)
        heap.push(load);

    AllocationResult result;

    result.remainingPower = availablePower;

    while (!heap.empty()) {

        Load current = heap.pop();

        if (current.power <= result.remainingPower) {

            current.active = true;

            result.allocated.push_back(current);

            result.totalPower += current.power;

            result.remainingPower -= current.power;

        } else {

            current.active = false;

            result.waiting.push_back(current);
        }
    }

    return result;
}

// ==========================================
// 10. KNAPSACK DYNAMIC PROGRAMMING
// ==========================================

int knapsack(
    const vector<int>& power,
    const vector<int>& value,
    int capacity
) {

    if (capacity < 0 || power.size() != value.size())
        throw invalid_argument("Invalid knapsack input");

    vector<int> dp(capacity + 1, 0);

    for (int i = 0; i < power.size(); i++) {

        if (power[i] <= 0 || value[i] < 0)
            throw invalid_argument("Invalid item");

        for (int w = capacity; w >= power[i]; w--) {

            dp[w] = max(
                dp[w],
                dp[w - power[i]] + value[i]
            );
        }
    }

    return dp[capacity];
}

#endif
