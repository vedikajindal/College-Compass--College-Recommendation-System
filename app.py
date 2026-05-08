import json
import os
import subprocess
import sys
from flask import Flask, render_template, request, jsonify

app = Flask(__name__)

BASE_DIR   = os.path.dirname(os.path.abspath(__file__))
_bin_name  = "algorithms.exe" if sys.platform == "win32" else "algorithms"
ALGO_BIN   = os.path.join(BASE_DIR, _bin_name)

CATEGORIES = ["GEN", "EWS", "OBC", "SC", "ST"]
BRANCHES   = ["CS","ECE","ME","CE","EE","BT","CH","IT","DS","AI",
              "AE","MT","MN","CB","BS","EEE","SS","BIO","BBA"]
CITY_NAMES = [
    "Delhi","Mumbai","Chennai","Kolkata","Bangalore",
    "Hyderabad","Pune","Ahmedabad","Jaipur","Lucknow",
    "Kanpur","Kharagpur","Roorkee","Pilani","Vellore",
    "Manipal","Coimbatore","Bhopal","Nagpur","Noida",
]

def call_cpp(*args):
    if not os.path.isfile(ALGO_BIN):
        raise RuntimeError(
            f"C++ binary not found at {ALGO_BIN}. "
            "Run: g++ -O2 -std=c++17 -o algorithms algorithms.cpp"
        )
    cmd = [ALGO_BIN] + [str(a) for a in args]
    result = subprocess.run(cmd, capture_output=True, text=True, timeout=10)
    if result.returncode != 0:
        raise RuntimeError(f"C++ error: {result.stderr.strip()}")
    return json.loads(result.stdout)

_colleges_cache = None

def get_colleges():
    global _colleges_cache
    if _colleges_cache is None:
        _colleges_cache = call_cpp("colleges")
    return _colleges_cache

@app.route("/")
def index():
    return render_template(
        "index.html",
        colleges=get_colleges(),
        cities=list(enumerate(CITY_NAMES)),
        branches=BRANCHES,
        categories=CATEGORIES,
    )
@app.route("/api/recommend", methods=["POST"])
def api_recommend():
    data = request.json

    profile = {
        "name":             data.get("name", ""),
        "jee_main_rank":    int(data.get("jee_main_rank", 0)),
        "jee_adv_rank":     int(data.get("jee_adv_rank", 0)),
        "bitsat_score":     int(data.get("bitsat_score", 0)),
        "category":         data.get("category", "GEN"),
        "gender":           data.get("gender", "MALE"),
        "home_state":       data.get("home_state", ""),
        "preferred_branch": data.get("preferred_branch", ""),
        "budget_lakhs":     float(data.get("budget_lakhs", 0)),
        "min_package":      float(data.get("min_package", 0)),
        "city_id":          int(data.get("city_id", 0)),
        "top_k":            int(data.get("top_k", 10)),
        "w_rank":           float(data.get("w_rank", 0.20)),
        "w_package":        float(data.get("w_package", 0.25)),
        "w_fees":           float(data.get("w_fees", 0.15)),
        "w_placement":      float(data.get("w_placement", 0.20)),
        "w_distance":       float(data.get("w_distance", 0.10)),
        "w_infra":          float(data.get("w_infra", 0.10)),
    }

    results = call_cpp("recommend", json.dumps(profile))
    return jsonify(results)
@app.route("/api/compare", methods=["POST"])
def api_compare():
    data = request.json
    result = call_cpp("compare", int(data["id1"]), int(data["id2"]))
    return jsonify(result)

@app.route("/api/sort", methods=["GET"])
def api_sort():
    criterion = request.args.get("by", "rank")
    result = call_cpp("sort", criterion)
    return jsonify(result)

@app.route("/api/autocomplete", methods=["GET"])
def api_autocomplete():
    prefix = request.args.get("q", "")
    result = call_cpp("autocomplete", prefix)
    return jsonify(result)

@app.route("/api/cutoffs", methods=["GET"])
def api_cutoffs():
    college_id = int(request.args.get("id", 0))
    exam_type  = request.args.get("exam", "adv")
    category   = request.args.get("cat", "GEN")
    result = call_cpp("cutoffs", college_id, exam_type, category)
    return jsonify(result)

@app.route("/api/colleges", methods=["GET"])
def api_colleges():
    return jsonify(get_colleges())

@app.errorhandler(Exception)
def handle_error(e):
    return jsonify({"error": str(e)}), 500

if __name__ == "__main__":
    if not os.path.isfile(ALGO_BIN):
        print(f"[WARNING] C++ binary '{ALGO_BIN}' not found.", file=sys.stderr)
        print("  Build it with:  g++ -O2 -std=c++17 -o algorithms algorithms.cpp", file=sys.stderr)
    else:
        print(f"[OK] C++ engine: {ALGO_BIN}")
    app.run(debug=True, port=5000)
