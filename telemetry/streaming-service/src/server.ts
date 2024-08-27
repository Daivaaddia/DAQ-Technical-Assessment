import { count } from "console";
import net from "net";
import { WebSocket, WebSocketServer } from "ws";
import { createUnzip } from "zlib";

interface VehicleData {
  battery_temperature: number;
  timestamp: number;
}

const TCP_PORT = 12000;
const WS_PORT = 8080;
const tcpServer = net.createServer();
const websocketServer = new WebSocketServer({ port: WS_PORT });

let countAnomaly = 0;

tcpServer.on("connection", (socket) => {
  console.log("TCP client connected");

  socket.on("data", (msg) => {
    console.log(`Received: ${msg.toString()}`);

    let jsonData: VehicleData;
    try {
      jsonData = JSON.parse(msg.toString());
    } catch (e) {
      jsonData = JSON.parse(msg.toString().substring(0, msg.length - 1));
    }

    if (jsonData.battery_temperature > 80 || jsonData.battery_temperature < 20) {
      countAnomaly++;
      console.log("Anomaly detected: " + countAnomaly)

      if (countAnomaly > 3) {
        console.log(jsonData.timestamp + ": Normal temperature exceeded more than 3 times in 5 seconds!")
      }

      setTimeout(() => {
        countAnomaly--;
      }, 5000)
    }

    // Send JSON over WS to frontend clients
    websocketServer.clients.forEach(function each(client) {
      if (client.readyState === WebSocket.OPEN) {
        client.send(msg.toString());
      }
    });
  });

  socket.on("end", () => {
    console.log("Closing connection with the TCP client");
  });

  socket.on("error", (err) => {
    console.log("TCP client error: ", err);
  });
});

websocketServer.on("listening", () =>
  console.log(`Websocket server started on port ${WS_PORT}`)
);

websocketServer.on("connection", async (ws: WebSocket) => {
  console.log("Frontend websocket client connected");
  ws.on("error", console.error);
});

tcpServer.listen(TCP_PORT, () => {
  console.log(`TCP server listening on port ${TCP_PORT}`);
});
