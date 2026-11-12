
const API = "/api";

// =====================================
// BACKEND API REQUEST
// =====================================

async function apiRequest(endpoint, options = {}) {

    const response = await fetch(API + endpoint, {
        ...options,
        headers: {
            "Content-Type": "application/json",
            ...options.headers
        }
    });

    const data = await response.json();

    if (!response.ok) {
        throw new Error(data.error || "Backend request failed");
    }

    return data;
}

// =====================================
// CHECK SERVER CONNECTION
// =====================================

async function checkConnection() {

    const connection =
        document.getElementById("connection");

    try {

        await apiRequest("/health");

        connection.textContent = "Connected";
        connection.style.color = "#4ade80";

    } catch (error) {

        connection.textContent = "Disconnected";
        connection.style.color = "#f87171";
    }
}

// =====================================
// LOAD DASHBOARD STATISTICS
// =====================================

async function loadDashboard() {

    try {

        const data = await apiRequest("/status");

        document.getElementById("roomsCount").textContent =
            data.rooms;

        document.getElementById("loadsCount").textContent =
            data.loads;

    } catch (error) {

        console.error("Dashboard error:", error);
    }
}

// =====================================
// SMART POWER ALLOCATION
// =====================================

async function runAllocation() {

    const button = document.querySelector(
        "#allocation button"
    );

    const power = Number(
        document.getElementById("powerInput").value
    );

    if (
        !Number.isInteger(power) ||
        power < 0 ||
        power > 1000000
    ) {

        alert("Enter a valid power value.");
        return;
    }

    button.disabled = true;
    button.textContent = "Processing...";

    try {

        const result = await apiRequest("/allocate", {

            method: "POST",

            body: JSON.stringify({
                availablePower: power
            })

        });

        document.getElementById("available").textContent =
            result.availablePower + " W";

        document.getElementById("allocated").textContent =
            result.allocatedPower + " W";

        document.getElementById("resultAllocated").textContent =
            result.allocatedPower + " W";

        document.getElementById("resultRemaining").textContent =
            result.remainingPower + " W";

        document.getElementById("allocationMessage").textContent =
            "Power allocation calculated successfully.";

        displayLoads(
            "activeLoads",
            result.allocated,
            "on"
        );

        displayLoads(
            "waitingLoads",
            result.waiting,
            "waiting"
        );

    } catch (error) {

        alert(error.message);

    } finally {

        button.disabled = false;
        button.textContent = "⚡ Run Allocation";
    }
}

// =====================================
// DISPLAY ELECTRICAL LOADS
// =====================================

function displayLoads(containerId, loads, statusClass) {

    const container =
        document.getElementById(containerId);

    container.innerHTML = "";

    if (loads.length === 0) {

        container.textContent = "No loads.";
        return;
    }

    loads.forEach(load => {

        const item = document.createElement("div");

        item.className = "load-item";

        const info = document.createElement("div");

        const title = document.createElement("strong");

        const details = document.createElement("small");

        title.textContent = load.name;

        details.textContent =
            `${load.room} | ${load.power} W | Priority ${load.priority}`;

        info.append(title, details);

        const status = document.createElement("span");

        status.className = statusClass;

        status.textContent = load.status;

        item.append(info, status);

        container.appendChild(item);
    });
}

// =====================================
// LOAD HOSTEL ROOMS
// =====================================

async function loadRooms() {

    try {

        const rooms = await apiRequest("/rooms");

        const table =
            document.getElementById("roomTable");

        table.innerHTML = "";

        rooms.forEach(room => {

            const tr = document.createElement("tr");

            [room.id, room.floor, room.consumption + " W"]
                .forEach(value => {

                    const td = document.createElement("td");

                    td.textContent = value;

                    tr.appendChild(td);
                });

            table.appendChild(tr);
        });

    } catch (error) {

        console.error("Room loading error:", error);
    }
}

// =====================================
// ADD NEW HOSTEL ROOM
// =====================================

async function addRoom() {

    const id = document.getElementById("roomId")
        .value.trim();

    const floor = Number(
        document.getElementById("roomFloor").value
    );

    const consumption = Number(
        document.getElementById("roomConsumption").value
    );

    if (
        !id ||
        !Number.isInteger(floor) ||
        floor < 1 ||
        floor > 100 ||
        !Number.isInteger(consumption) ||
        consumption < 0
    ) {

        alert("Enter valid room details.");
        return;
    }

    try {

        await apiRequest("/rooms", {

            method: "POST",

            body: JSON.stringify({
                id,
                floor,
                consumption
            })

        });

        alert("Room added successfully!");

        document.getElementById("roomId").value = "";
        document.getElementById("roomFloor").value = "";
        document.getElementById("roomConsumption").value = "";

        await loadRooms();

        await loadDashboard();

    } catch (error) {

        alert(error.message);
    }
}

// =====================================
// HOSTEL GRAPH - BFS
// =====================================

async function loadNetwork() {

    try {

        const data = await apiRequest("/network");

        document.getElementById("networkResult")
            .textContent =
            "BFS Traversal: " + data.nodes.join(" → ");

    } catch (error) {

        alert(error.message);
    }
}

// =====================================
// INITIALIZE DASHBOARD
// =====================================

async function initialize() {

    await checkConnection();

    await loadDashboard();

    await loadRooms();

    // Initial demonstration allocation
    await runAllocation();
}

document.addEventListener(
    "DOMContentLoaded",
    initialize
);
