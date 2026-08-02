from flask import Flask, request, jsonify, render_template

app = Flask(__name__)

# Variables globales pour stocker temporairement les données
intrusion = False
@app.route('/api/data', methods=['POST'])
def receive_data():
    global intrusion
    # Récupération des données JSON envoyées dans le corps de la requête
    data = request.get_json()
    
    if not data:
        return jsonify({"status": "error", "message": "Aucun corps JSON fourni"}), 400
    
    intrusion = data.get('intrusion')
    
    
    
    print(f"Données reçues sur le serveur -  etat: {intrusion}")
    
    # Traitement ou sauvegarde des données ici...
    
    return jsonify({
        "status": "success",
        "message": "Données bien enregistrées !",
        "recu": {"intrusion": intrusion}
    }), 200


@app.route("/home")
def home():
    return render_template("index.html", message = "intrusion dans la piece detectez merci d'envoyer des renforts.")
    


# Nouvelle route pour alimenter le script JS du dashboard en direct
@app.route('/api/last-data', methods=['GET'])
def get_last_data():
    return jsonify({
        "intrusion": intrusion
    }), 200


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)