import { useState, useEffect } from "react";
import LiveValue from "./live_value";
import RedbackLogo from "./redback_logo.jpg";
import "./App.css";
import useWebSocket, { ReadyState } from "react-use-websocket";
import Logs from "./logs";

const WS_URL = "ws://localhost:8080";

interface VehicleData {
  battery_temperature: number;
  timestamp: number;
  errorMsg?: string;
}

interface LogData {
  timestamp: number;
  msg: string;
}

function App() {
  const [temperature, setTemperature] = useState<number>(0);
  const [logs, setLogs] = useState<Array<LogData>>([]);
  const {
    lastJsonMessage,
    readyState,
  }: { lastJsonMessage: VehicleData | null; readyState: ReadyState } =
    useWebSocket(WS_URL, {
      share: false,
      shouldReconnect: () => true,
    });

  useEffect(() => {
    switch (readyState) {
      case ReadyState.OPEN:
        console.log("Connected to streaming service");
        break;
      case ReadyState.CLOSED:
        console.log("Disconnected from streaming service");
        break;
      default:
        break;
    }
  }, [readyState]);

  useEffect(() => {
    console.log("Received: ", lastJsonMessage);
    if (lastJsonMessage === null) {
      return;
    }
    setTemperature(lastJsonMessage["battery_temperature"]);

    if (lastJsonMessage["errorMsg"]) {
      const newLog: LogData = {timestamp: lastJsonMessage["timestamp"], msg: lastJsonMessage["errorMsg"]};
      setLogs(prevLogs => [newLog, ...prevLogs]);
    }
  }, [lastJsonMessage]);

  return (
    <div className="App">
      <header className="App-header">
        <Logs logs={logs} />
        <div className="data-display">
          <img
            src={RedbackLogo}
            className="redback-logo"
            alt="Redback Racing Logo"
          />
          <p className="value-title">Live Battery Temperature</p>
          <LiveValue temp={temperature} />
        </div>
      </header>
    </div>
  );
}

export default App;
