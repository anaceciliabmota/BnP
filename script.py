import os
import subprocess

# Caminho para o diretório com os arquivos
dir_path = "instances2"
# Caminho para o executável
executable_path = "./bpp"
# Arquivo de saída
output_file = "resultados.txt"

# Verificar se o diretório existe
if not os.path.isdir(dir_path):
    raise FileNotFoundError(f"O diretório {dir_path} não foi encontrado.")

# Abrir o arquivo de saída em modo de escrita
with open(output_file, "w") as output:
    # Listar todos os arquivos no diretório
    for file_name in os.listdir(dir_path):
        file_path = os.path.join(dir_path, file_name)
        
        # Verificar se é um arquivo
        if os.path.isfile(file_path):
            # Exibir no terminal o nome do arquivo sendo processado
            print(f"Processando arquivo: {file_name}")

            try:
                # Executar o programa com o arquivo como argumento, com limite de tempo
                result = subprocess.run([executable_path, file_path], 
                                        capture_output=True, text=True, timeout=300)

                # Escrever o nome do arquivo e a saída no arquivo de saída
                output.write(f"Arquivo: {file_name}\n")
                output.write(f"{result.stdout}")
                output.write(f"Erros:\n{result.stderr}")
                output.write("-" * 40 + "\n")

            except subprocess.TimeoutExpired:
                # Registrar se o programa ultrapassou o limite de tempo
                output.write(f"Erro: Tempo limite excedido ao processar {file_name}\n")
                output.write("-" * 40 + "\n")

            except Exception as e:
                # Registrar qualquer outro erro ao executar o programa
                output.write(f"Erro ao processar {file_name}: {e}\n")
                output.write("-" * 40 + "\n")

print(f"Processamento concluído. Resultados salvos em {output_file}.")
