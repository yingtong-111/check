from flask import Flask, request, jsonify
import sqlite3

app = Flask(__name__)

def query_database_by_product_name(product_name):
    conn = sqlite3.connect('market_database.db')
    cursor = conn.cursor()
    cursor.execute("SELECT * FROM products WHERE ProductName = ?", (product_name,))
    product = cursor.fetchone()
    conn.close()
    if product:
        return {
            "ProductName": product[0],
            "Block": product[1],
            "Shelf": product[2],
            "Level": product[3],
            "Price": product[4],
            "Unit": product[5],
            "Image": product[6]  # Assuming there is an image field in your database
        }
    else:
        return None

@app.route('/upload', methods=['POST'])
def upload_image():
    file = request.files['image']
    if file:
        # Remove file extension and query database
        product_info = query_database_by_product_name(file.filename.split('.')[0])
        if product_info:
            return jsonify(product_info)
        else:
            return jsonify({"error": "No matching product found in the database."})
    else:
        return jsonify({"error": "No file uploaded."})

if __name__ == '__main__':
    app.run(debug=True)
