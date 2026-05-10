from flask import Flask, request, jsonify
import whisper
import os

app = Flask(__name__)

print("Loading Whisper model...")
model = whisper.load_model("base")
print("Whisper model loaded.")

@app.route("/health", methods=["GET"])
def health():
    return jsonify({"status": "ok"})

@app.route("/transcribe", methods=["POST"])
def transcribe():
    if "audio" not in request.files:
        return jsonify({
            "success": False,
            "error": "No audio file uploaded"
        }), 400

    audio_file = request.files["audio"]

    os.makedirs("uploads", exist_ok=True)

    file_path = os.path.join("uploads", audio_file.filename)
    audio_file.save(file_path)

    try:
        result = model.transcribe(file_path)

        return jsonify({
            "success": True,
            "transcription": result["text"]
        })

    except Exception as e:
        return jsonify({
            "success": False,
            "error": str(e)
        }), 500

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=6000)