from flask import Flask, request, jsonify, send_from_directory, render_template, redirect, url_for
from flask_cors import CORS
import cv2
import numpy as np
import pytesseract
from PIL import Image
import io
import sqlite3

app = Flask(__name__, static_folder='.', static_url_path='')
CORS(app)

# Route to serve the search page
@app.route('/')
def index():
    return send_from_directory('.', 'searchpage.html')

# Function to extract text from image using Tesseract
def extract_text(image):
    pytesseract.pytesseract.tesseract_cmd = r'path_to_tesseract.exe'  # Update this path to your Tesseract installation
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    text = pytesseract.image_to_string(gray, lang='eng')
    return text.strip()

# Function to query the database for product information
def query_database(text):
    conn = sqlite3.connect('market_database.db')
    cursor = conn.cursor()
    cursor.execute("SELECT * FROM products WHERE ProductName LIKE ?", (f"%{text}%",))
    products = cursor.fetchall()
    conn.close()
    return [{'ProductName': prod[0], 'Block': prod[1], 'Shelf': prod[2], 'Level': prod[3], 'Price': prod[4], 'Unit': prod[5]} for prod in products]



# 修改upload_image来直接返回产品数据的JSON，而不是重定向
@app.route('/upload', methods=['POST'])
def upload_image():
    print("Received upload request")  # 检查是否收到请求
    if 'image' not in request.files:
        return jsonify({'error': 'No image part'})
    
    file = request.files['image']
    print("File received:", file.filename)  # 打印文件名以确认文件已收到
    if file.filename == '':
        return jsonify({'error': 'No selected file'})
    
    if file:
        in_memory_file = io.BytesIO()
        file.save(in_memory_file)
        data = np.frombuffer(in_memory_file.getvalue(), dtype=np.uint8)
        image = cv2.imdecode(data, cv2.IMREAD_COLOR)
        
        text = extract_text(image)
        print("Extracted text:", text)  # 打印提取的文字
        if text:
            products = query_database(text)
            print("Database response:", products)  # 查看数据库响应
            return jsonify({'products': products})
        else:
            return jsonify({'error': 'No text found'})

    return jsonify({'error': 'Unhandled case'})




# Route to display results page
@app.route('/results')
def results():
    products = request.args.get('products')
    return render_template('page.html', products=products)

if __name__ == '__main__':
    app.run(debug=True)
