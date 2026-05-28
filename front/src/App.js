import { useEffect, useState, useRef } from "react";

function createInitialState(size) {
  const grid = Array(size * size).fill(0);

  const snake = { parts: [{x: 2, y: 2}], dir: 1, len: 1 };
  grid[2 + 2 * size] = 1;
  grid[3 + 2 * size] = -1;

  return {
    size: size,
    grid,
    snake,
    dir: 3 // RIGHT
  };
}

export default function App() {
  const [status, setStatus] = useState(0);
  const [speed, setSpeed] = useState(2); // cells per second
  const [size, setSize] = useState(4);
  const [state, setState] = useState(() => createInitialState(size));
  const stateRef = useRef(state);

  let busyRef = useRef(false);
  async function sendStep(dir) {
    if (busyRef.current) return;
    busyRef.current = true;

    const res = await fetch("http://localhost:8888", {
      method: "POST",
      headers: {
        "Content-Type": "application/json"
      },
      body: JSON.stringify({
        ...stateRef.current,
        dir
      })
    });

    const data = await res.json();

    setState({
      size: data.size,
      grid: data.grid,
      snake: data.snake,
      len: data.len,
      dir: data.snake.dir
    });

    setStatus(data.status);
    busyRef.current = false;
  }

  useEffect(() => {
    stateRef.current = state;
  }, [state]);

  useEffect(() => {
    function handleKey(e) {
      if (e.key === "w") setState(s => ({ ...s, dir: 4 }));
      if (e.key === "s") setState(s => ({ ...s, dir: 2 }));
      if (e.key === "a") setState(s => ({ ...s, dir: 3 }));
      if (e.key === "d") setState(s => ({ ...s, dir: 1 }));
    }

    window.addEventListener("keydown", handleKey);
    return () => window.removeEventListener("keydown", handleKey);
  }, []);

  useEffect(() => {
    if (status !== 0) return;

    let stopped = false;

    async function loop() {
      while (!stopped) {
        const start = Date.now();

        await sendStep(stateRef.current.dir);

        const elapsed = Date.now() - start;
        const delay = Math.max(0, 1000 / speed - elapsed);

        await new Promise(r => setTimeout(r, delay));
      }
    }

    loop();

    return () => {
      stopped = true;
    };
  }, [speed, status]);

  return (
    <div style={{ textAlign: "center", marginTop: 40 }}>
      <h1>Snake</h1>

      <Grid grid={state.grid} size={state.size} />

      <p>Status: {status}</p>

      <div style={{ marginTop: 20 }}>
        <label>
          Speed: {speed}
          <input
            type="range"
            min="1"
            max="10"
            value={speed}
            onChange={(e) => setSpeed(Number(e.target.value))}
          />
        </label>
      </div>

      <div>
        <label>
          Grid Size: {size}
          <input
            type="number"
            min="4"
            max="20"
            value={size}
            onChange={(e) => {
              const newSize = Number(e.target.value);
              setSize(newSize);
              setState(createInitialState(newSize));
            }}
          />
        </label>
      </div>

      <div>
        <button onClick={() => sendStep(4)}>↑</button>
        <button onClick={() => sendStep(3)}>←</button>
        <button onClick={() => sendStep(2)}>↓</button>
        <button onClick={() => sendStep(1)}>→</button>
      </div>
    </div>
  );
}

function Grid({ grid, size }) {
  return (
    <div
      style={{
        display: "grid",
        gridTemplateColumns: `repeat(${size}, 40px)`,
        gap: 2,
        justifyContent: "center"
      }}
    >
      {grid.map((cell, i) => (
        <Cell key={i} value={cell} />
      ))}
    </div>
  );
}

function Cell({ value }) {
  let bg = "#eee";

  if (value > 0) bg = "green";     // snake
  if (value < 0) bg = "red";       // apple

  return (
    <div
      style={{
        width: 40,
        height: 40,
        backgroundColor: bg,
        border: "1px solid #ccc"
      }}
    />
  );
}