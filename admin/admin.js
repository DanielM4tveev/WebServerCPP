// Функция для получения статуса сервера
function fetchStatus() {
    fetch('/status') // Предполагается, что сервер поддерживает этот эндпоинт
        .then(response => response.json())
        .then(data => {
            document.getElementById('status-info').innerHTML = `
                <p>Uptime: ${data.uptime}</p>
                <p>Connections: ${data.connections}</p>
            `;
        })
        .catch(error => console.error('Error fetching status:', error));
}

// Функция для загрузки файла на сервер
function uploadFile() {
    const fileInput = document.getElementById('file-input');
    const file = fileInput.files[0];

    if (!file) {
        alert('Please select a file.');
        return;
    }

    const formData = new FormData();
    formData.append('file', file);

    fetch('/upload', { // Предполагается, что сервер поддерживает этот эндпоинт
        method: 'POST',
        body: formData
    })
    .then(response => response.json())
    .then(data => {
        document.getElementById('upload-status').innerText = data.message;
    })
    .catch(error => {
        document.getElementById('upload-status').innerText = 'Error uploading file.';
        console.error('Error uploading file:', error);
    });
}

// Инициализация панели
document.addEventListener('DOMContentLoaded', () => {
    fetchStatus();
});
