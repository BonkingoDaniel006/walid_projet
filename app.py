from flask import Flask, request, jsonify, render_template

app = Flask(__name__)

# État global du système
intrusion = False
actif = False
distance_capteur = 0.0


@app.route('/')
@app.route('/home')
def home():
    return render_template('index.html')


@app.route('/api/data', methods=['POST'])
def receive_data():
    global intrusion, actif, distance_capteur

    data = request.get_json(silent=True)
    if not data:
        return jsonify({"status": "error", "message": "Aucun corps JSON valide fourni"}), 400

    # Lecture des données du capteur envoyées par l'ESP32
    intrusion = bool(data.get('intrusion', False))
    distance_capteur = float(data.get('distance', 0.0))

    print(f"Données ESP32 reçues - Intrusion: {intrusion}, Distance: {distance_capteur}cm, Actif: {actif}")

    # Renvoi de l'état 'actif' à l'ESP32 pour qu'il sache s'il doit tourner/mesurer
    return jsonify({
        "status": "success",
        "actif": actif,
        "intrusion": intrusion,
        "distance": distance_capteur
    }), 200


@app.route('/api/last-data', methods=['GET'])
def get_last_data():
    return jsonify({
        "intrusion": intrusion,
        "actif": actif,
        "distance": distance_capteur
    }), 200


@app.route('/status', methods=['GET'])
def status():
    return jsonify({
        "actif": actif,
        "alarme": intrusion,
        "distance": distance_capteur  # Renvoie la VRAIE distance à l'interface Web
    }), 200


@app.route('/gauche')
def gauche():
    print("Action : Gauche")
    return jsonify({"status": "ok", "action": "gauche"})


@app.route('/droite')
def droite():
    print("Action : Droite")
    return jsonify({"status": "ok", "action": "droite"})


@app.route('/stop')
def stop():
    global actif, intrusion, distance_capteur
    actif = False
    intrusion = False
    distance_capteur = 0.0
    return jsonify({"status": "ok", "action": "stop"})


@app.route('/start')
def start():
    global actif
    actif = True
    return jsonify({"status": "ok", "action": "start"})


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)