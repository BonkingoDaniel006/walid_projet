from flask import Flask, request, jsonify, render_template

app = Flask(__name__)

# État global de détection
intrusion = False
actif = False


@app.route('/')
@app.route('/home')
def home():
    return render_template('index.html')


@app.route('/api/data', methods=['POST'])
def receive_data():
    global intrusion, actif

    data = request.get_json(silent=True)
    if not data:
        return jsonify({"status": "error", "message": "Aucun corps JSON valide fourni"}), 400

    intrusion = bool(data.get('intrusion', False))
    actif = bool(data.get('actif', True))

    print(f"Données reçues sur le serveur - intrusion: {intrusion}, actif: {actif}")

    return jsonify({
        "status": "success",
        "message": "Données bien enregistrées !",
        "recu": {"intrusion": intrusion, "actif": actif}
    }), 200


@app.route('/api/last-data', methods=['GET'])
def get_last_data():
    return jsonify({
        "intrusion": intrusion,
        "actif": actif
    }), 200


@app.route('/status', methods=['GET'])
def status():
    return jsonify({
        "actif": actif,
        "alarme": intrusion,
        "distance": 12 if intrusion else 0
    }), 200


@app.route('/gauche')
def gauche():
    return jsonify({"status": "ok", "action": "gauche"})


@app.route('/droite')
def droite():
    return jsonify({"status": "ok", "action": "droite"})


@app.route('/stop')
def stop():
    global actif
    actif = False
    return jsonify({"status": "ok", "action": "stop"})


@app.route('/start')
def start():
    global actif
    actif = True
    return jsonify({"status": "ok", "action": "start"})


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)