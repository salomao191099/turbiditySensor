import http.server
import socketserver
import os
import requests

PORT = 8002  # Você pode alterar a porta se desejar
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
                #dados_formatados = ''.join(filter(str.isdigit, dados))
                dados_formatados = dados
                # Retirando o valor de turbidez
                # Percorre a string a partir do índice 21
                # Variável para armazenar os valores desejados
                valores_selecionados = ""
                primeiro_valor = True  # Flag para identificar o primeiro valor
                for valor in dados_formatados[22:]:
                    if primeiro_valor:
                        primeiro_valor = False  # Marca que o primeiro valor já foi encontrado
                        valores_selecionados += valor  # Adiciona o valor à nova string

                    elif valor == '.':  # Verifica se o valor é o caractere '.'
                        indice = dados_formatados.index(valor)
                        break
                    else:
                        valores_selecionados += valor  # Adiciona o valor à nova string
                dados_formatadosTurbidez = valores_selecionados
                dados_formatadosTemperatura = dados_formatados[indice+4:indice+9]
                dados_formatadosTemperatura = ''.join(filter(lambda x: x.isdigit() or x == '.', dados_formatadosTemperatura))
                # Retorna os dados obtidos
                self.send_response(200)
                self.send_header('Content-type', 'text/plain')
                self.end_headers()
                resposta = f"{dados_formatadosTurbidez} {dados_formatadosTemperatura}"
                self.wfile.write(resposta.encode())
                #print(dados_formatadosTurbidez)
                print(resposta)
            else:
                # Se a solicitação falhar, envia uma mensagem de erro
                self.send_error(500, 'Erro ao obter os dados')
        else:
            # Continua com o tratamento padrão
            super().do_GET()

class ReusableTCPServer(socketserver.TCPServer):
    allow_reuse_address = True

os.chdir("templates")  # Mude o diretório de trabalho para a pasta templates

# Cria o servidor reutilizável
with ReusableTCPServer(("", PORT), MyHandler) as httpd:
    print("Servidor HTTP rodando na porta", PORT)
    httpd.serve_forever()
