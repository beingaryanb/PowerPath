#include "httplib.h"
#include "powerpath.h"
#include <cstdlib>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

// ==========================================
// GLOBAL OBJECTS
// ==========================================

RoomManager roomManager;
HostelGraph hostelGraph;
HostelTree hostelTree;

RestorationQueue<string> restorationQueue;

vector<Load> hostelLoads = {

    {1, "Emergency Lights", "COMMON", 200, 1},
    {2, "Security System", "COMMON", 100, 1},
    {3, "Water Pump", "COMMON", 500, 2},
    {4, "Floor 1 Fans", "101", 400, 3},
    {5, "Floor 1 Lights", "102", 200, 3},
    {6, "Floor 2 Fans", "201", 400, 3},
    {7, "Floor 2 Lights", "202", 200, 3},
    {8, "Optional Heaters", "301", 600, 4}

};

// ==========================================
// INITIALIZE HOSTEL
// ==========================================

void initializeHostel() {

    // Rooms

    roomManager.addRoom({"101", 1, 250});
    roomManager.addRoom({"102", 1, 300});
    roomManager.addRoom({"103", 1, 200});

    roomManager.addRoom({"201", 2, 350});
    roomManager.addRoom({"202", 2, 400});
    roomManager.addRoom({"203", 2, 250});

    roomManager.addRoom({"301", 3, 300});
    roomManager.addRoom({"302", 3, 200});

    // Tree structure

    hostelTree.addFloor("Floor 1");
    hostelTree.addFloor("Floor 2");
    hostelTree.addFloor("Floor 3");

    hostelTree.addRoom("Floor 1", "Room 101");
    hostelTree.addRoom("Floor 1", "Room 102");
    hostelTree.addRoom("Floor 1", "Room 103");

    hostelTree.addRoom("Floor 2", "Room 201");
    hostelTree.addRoom("Floor 2", "Room 202");
    hostelTree.addRoom("Floor 2", "Room 203");

    hostelTree.addRoom("Floor 3", "Room 301");
    hostelTree.addRoom("Floor 3", "Room 302");

    // Graph structure

    hostelGraph.addConnection("Main Supply", "Floor 1");
    hostelGraph.addConnection("Main Supply", "Floor 2");
    hostelGraph.addConnection("Main Supply", "Floor 3");

    hostelGraph.addConnection("Floor 1", "Room 101");
    hostelGraph.addConnection("Floor 1", "Room 102");
    hostelGraph.addConnection("Floor 1", "Room 103");

    hostelGraph.addConnection("Floor 2", "Room 201");
    hostelGraph.addConnection("Floor 2", "Room 202");
    hostelGraph.addConnection("Floor 2", "Room 203");

    hostelGraph.addConnection("Floor 3", "Room 301");
    hostelGraph.addConnection("Floor 3", "Room 302");
}

// ==========================================
// MAIN FUNCTION
// ==========================================

int main() {

    initializeHostel();

    httplib::Server server;

    // ======================================
    // 1. BACKEND HEALTH CHECK
    // ======================================

    server.Get("/api/health",
        [](const httplib::Request& req,
           httplib::Response& res) {

            json response = {
                {"status", "online"},
                {"project", "PowerPath"},
                {"language", "C++"}
            };

            res.set_content(
                response.dump(),
                "application/json"
            );
        }
    );

    // ======================================
    // 2. GET ALL ROOMS
    // ======================================

    server.Get("/api/rooms",
        [](const httplib::Request& req,
           httplib::Response& res) {

            json result = json::array();

            for (const auto& room : roomManager.getAll()) {

                result.push_back({

                    {"id", room.id},
                    {"floor", room.floor},
                    {"consumption", room.consumption}

                });
            }

            res.set_content(
                result.dump(),
                "application/json"
            );
        }
    );

    // ======================================
    // 3. ADD NEW ROOM
    // ======================================

    server.Post("/api/rooms",
        [](const httplib::Request& req,
           httplib::Response& res) {

            try {

                auto body = json::parse(req.body);

                string id = body.at("id").get<string>();

                int floor = body.at("floor").get<int>();

                int consumption =
                    body.at("consumption").get<int>();

                if (
                    id.empty() ||
                    floor < 1 ||
                    floor > 100 ||
                    consumption < 0 ||
                    consumption > 100000
                ) {

                    res.status = 400;

                    res.set_content(
                        R"({"error":"Invalid room details"})",
                        "application/json"
                    );

                    return;
                }

                if (roomManager.exists(id)) {

                    res.status = 409;

                    res.set_content(
                        R"({"error":"Room already exists"})",
                        "application/json"
                    );

                    return;
                }

                roomManager.addRoom({
                    id,
                    floor,
                    consumption
                });

                hostelTree.addRoom(
                    "Floor " + to_string(floor),
                    "Room " + id
                );

                res.set_content(
                    R"({"message":"Room added successfully"})",
                    "application/json"
                );

            }
            catch (...) {

                res.status = 400;

                res.set_content(
                    R"({"error":"Invalid JSON request"})",
                    "application/json"
                );
            }
        }
    );

    // ======================================
    // 4. SMART POWER ALLOCATION
    // ======================================

    server.Post("/api/allocate",
        [](const httplib::Request& req,
           httplib::Response& res) {

            try {

                auto body = json::parse(req.body);

                int availablePower =
                    body.at("availablePower").get<int>();

                if (
                    availablePower < 0 ||
                    availablePower > 1000000
                ) {

                    res.status = 400;

                    res.set_content(
                        R"({"error":"Invalid power supply"})",
                        "application/json"
                    );

                    return;
                }

                auto result = allocatePower(
                    hostelLoads,
                    availablePower
                );

                // Reset restoration queue

                while (!restorationQueue.empty())
                    restorationQueue.pop();

                json allocated = json::array();

                json waiting = json::array();

                // Allocated loads

                for (const auto& load : result.allocated) {

                    allocated.push_back({

                        {"name", load.name},
                        {"room", load.room},
                        {"power", load.power},
                        {"priority", load.priority},
                        {"status", "ON"}

                    });
                }

                // Waiting loads

                for (const auto& load : result.waiting) {

                    waiting.push_back({

                        {"name", load.name},
                        {"room", load.room},
                        {"power", load.power},
                        {"priority", load.priority},
                        {"status", "WAITING"}

                    });

                    restorationQueue.push(load.name);
                }

                json response = {

                    {"availablePower", availablePower},

                    {"allocatedPower", result.totalPower},

                    {"remainingPower", result.remainingPower},

                    {"allocated", allocated},

                    {"waiting", waiting}

                };

                res.set_content(
                    response.dump(),
                    "application/json"
                );

            }
            catch (const exception& e) {

                res.status = 400;

                json error = {
                    {"error", e.what()}
                };

                res.set_content(
                    error.dump(),
                    "application/json"
                );
            }
        }
    );

    // ======================================
    // 5. RESTORATION QUEUE
    // ======================================

    server.Get("/api/restoration",
        [](const httplib::Request& req,
           httplib::Response& res) {

            json result = restorationQueue.snapshot();

            res.set_content(
                result.dump(),
                "application/json"
            );
        }
    );

    // ======================================
    // 6. HOSTEL GRAPH - BFS
    // ======================================

    server.Get("/api/network",
        [](const httplib::Request& req,
           httplib::Response& res) {

            vector<string> traversal =
                hostelGraph.BFS("Main Supply");

            json response = {

                {"algorithm", "BFS"},

                {"nodes", traversal}

            };

            res.set_content(
                response.dump(),
                "application/json"
            );
        }
    );

    // ======================================
    // 7. DASHBOARD STATUS
    // ======================================

    server.Get("/api/status",
        [](const httplib::Request& req,
           httplib::Response& res) {

            json response = {

                {"hostel", "PowerPath Hostel"},

                {"floors", 3},

                {"rooms", roomManager.getAll().size()},

                {"loads", hostelLoads.size()},

                {"engine", "C++17"},

                {"status", "Running"}

            };

            res.set_content(
                response.dump(),
                "application/json"
            );
        }
    );

    // ======================================
    // 8. SERVE FRONTEND
    // ======================================

    server.set_mount_point(
        "/",
        "../../frontend"
    );

    // ======================================
    // START SERVER
    // ======================================

    cout << "\n";
    cout << "====================================\n";
    cout << "       POWERPATH BACKEND\n";
    cout << "====================================\n";

    cout << "Server: http://localhost:8080\n";

    cout << "Backend: C++17\n";

    cout << "ADS Engine: ACTIVE\n";

    cout << "====================================\n";

    const char* portEnv = std::getenv("PORT");
int port = portEnv ? std::stoi(portEnv) : 8080;

server.listen("0.0.0.0", port);

    return 0;
}
