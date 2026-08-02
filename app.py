from flask import Flask, request, jsonify, render_template

app = Flask(__name__)

nom = None
age = None

@app.route('/api/data', methods=['POST'])
def receive_data():
    global nom, age
    # Récupération des données JSON envoyées dans le corps de la requête
    data = request.get_json()
    
    if not data:
        return jsonify({"status": "error", "message": "Aucun corps JSON fourni"}), 400
    
    nom = data.get('nom')
    age = data.get('age')
    
    print(f"Données reçues sur le serveur - Nom: {nom}, Âge: {age}")
    
    # Traitement ou sauvegarde des données ici...
    
    
    return jsonify({
        "status": "success",
        "message": "Données bien enregistrées !",
        "recu": {"nom": nom, "age": age}
    }), 200


@app.route("/home")
def home():

    return render_template("index.html", age = age, nom = nom)
    
if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)