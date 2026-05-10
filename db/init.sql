CREATE TABLE IF NOT EXISTS stations (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    stream_url TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS recordings (
    id INT AUTO_INCREMENT PRIMARY KEY,

    station_name VARCHAR(100) NOT NULL,

    start_time DATETIME NOT NULL,
    end_time DATETIME NOT NULL,

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

    transcription LONGTEXT,

    file_path TEXT NOT NULL,

    duration_seconds INT,
    
    FULLTEXT(transcription)
);