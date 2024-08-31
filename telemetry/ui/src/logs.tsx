interface LogsProps {
    logs: {
        timestamp: number;
        msg: string;
    }[];
}

function formatTimestamp(timestamp: number) {
    const timeStr = new Date(timestamp).toTimeString();
    return timeStr.split(' ')[0];
}

function Logs({ logs }: LogsProps) {
    return (
      <div className="log-section">
        <h2>Logs</h2>
        <div className="log-list">
            {logs.map(log => (
                <div className="log-container">
                    {formatTimestamp(log.timestamp) + ' - ' + log.msg}
                </div>
            ))}
        </div>
      </div>
    );
  }
  
  export default Logs;