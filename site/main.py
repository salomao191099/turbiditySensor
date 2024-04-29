import http.server
import socketserver
import os
import requests

PORT = 8000  # Você pode alterar a porta se desejar
Handler = http.server.SimpleHTTPRequestHandler

class MyHandler(Handler):
    def do_GET(self):
        if self.path == '/dados':
            # Faz a solicitação para obter os dados
            response = requests.get("https://dados.pagekite.me/obter-dados")
            if response.status_code == 200:
                # Lê os dados da resposta
                dados = response.text
                # Remove todos os caracteres desnecessários
                dados_formatados = ''.join(filter(str.isdigit, dados))
                # Remove 3 zeros
                dados_formatados = dados_formatados[:-3]
                # Retorna os dados obtidos
                self.send_response(200)
                self.send_header('Content-type', 'text/plain')
                self.end_headers()
                self.wfile.write(dados_formatados.encode())
                print(dados_formatados)
            else:
                # Se a solicitação falhar, envia uma mensagem de erro
                self.send_error(500, 'Erro ao obter os dados')
        else:
            # Continua com o tratamento padrão
            super().do_GET()

os.chdir("templates")  # Mude o diretório de trabalho para a pasta templates

with socketserver.TCPServer(("", PORT), MyHandler) as httpd:
    print("Servidor HTTP rodando na porta", PORT)
    httpd.serve_forever()
