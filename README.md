# 🧭 College Compass — College Recommendation System

> An intelligent, multi-criteria college recommendation engine for JEE / BITSAT aspirants, powered by a C++ algorithm backend and a Python Flask web interface.

---
## Features

- 🎯 **Personalized Recommendations** — Filters colleges by JEE Advanced rank, JEE Main rank, BITSAT score, category (GEN/EWS/OBC/SC/ST), gender, home state, and preferred branch
- ⚖️ **Weighted Scoring** — Tune importance of rank, avg package, fees, placement %, distance, and infrastructure
- 🔍 **Autocomplete Search** — Trie-based prefix search for fast college name lookup
- 📊 **College Comparison** — Side-by-side comparison of any two colleges
- 🔢 **Flexible Sorting** — Sort all colleges by NIRF rank, package, fees, or placement
- 📋 **Branch-wise Cutoffs** — View opening/closing ranks per branch, category, and exam type
- 🗺️ **Distance-Aware** — Dijkstra's algorithm on a city graph to factor in travel distance from your home city
- 👩 **Female Supernumerary Support** — Applies rank relaxation for female candidates at IITs
- 🏠 **Home/Outside State Quotas** — Correctly handles NIT home-state and outside-state cutoff differences

---

## Tech Stack

| Layer | Technology |
|-------|-----------|
| Algorithm Engine | C++17 (compiled binary) |
| Web Backend | Python 3, Flask |
| Frontend | HTML, CSS, JavaScript |
| Data Layer | Hardcoded college dataset in C++ (no external DB) |
| IPC | subprocess + JSON over stdout |

---
## Getting Started

### Prerequisites

- Python 3.8+
- Flask (`pip install flask`)
- g++ with C++17 support

### 1. Clone the Repository

```bash
git clone https://github.com/vedikajindal/college-compass-college-recommendation-system.git
cd college-compass
```

### 2. Compile the C++ Engine

**Linux / macOS:**
```bash
g++ -O2 -std=c++17 -o algorithms algorithms.cpp
```

**Windows:**
```bash
g++ -O2 -std=c++17 -o algorithms.exe algorithms.cpp
```

### 3. Install Python Dependencies

```bash
pip install flask
```

### 4. Run the App

```bash
python app.py
```

Open your browser at `http://localhost:5000`.

---
## Project Structure

```
college-compass/
├── algorithms.cpp        # Core C++ engine — all data structures, datasets & algorithms
├── algorithms            # Compiled binary for Linux/macOS (build it yourself)
├── algorithms.exe        # Compiled binary for Windows (build it yourself)
├── app.py                # Flask web server & REST API routes
├── templates/
│   └── index.html        # Frontend UI
└── .gitignore
```

---
## References

1. JoSAA Official Cutoff Data — https://josaa.nic.in
2. NIRF Rankings — https://www.nirfindia.org
3. Flask Documentation — https://flask.palletsprojects.com
4. C++ STL Reference — https://en.cppreference.com

