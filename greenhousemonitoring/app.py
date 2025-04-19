
from flask import Flask, render_template, request, jsonify, redirect
import pyrebase

app = Flask(__name__)

# Firebase Configuration
firebaseConfig = {
  apiKey: "AIzaSy....",
  authDomain: "your-project-id.firebaseapp.com",
  databaseURL: "https://your-project-id.firebaseio.com",
  projectId: "your-project-id",
  storageBucket: "your-project-id.appspot.com",
  messagingSenderId: "123456789",
  appId: "1:123456789:web:abcd1234"
}

firebase = pyrebase.initialize_app(firebaseConfig)
db = firebase.database()

@app.route('/')
def index():
    data = db.child("Greenhouse").get().val()
    relay = db.child("relay").get().val()
    return render_template("index.html", data=data, relay=relay)

@app.route('/toggle_relay', methods=['POST'])
def toggle_relay():
    current = db.child("relay").get().val()
    new_state = 0 if current == 1 else 1
    db.child("relay").set(new_state)
    return redirect('/')

@app.route('/data')
def data():
    data = db.child("Greenhouse").get().val()
    return jsonify(data)

@app.route('/about')
def about():
    return render_template("about.html")

@app.route('/contact')
def contact():
    return render_template("contact.html")

if __name__ == '__main__':
    app.run(debug=True)
