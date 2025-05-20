import re

def parse_primeiro_arquivo(caminho):
    resultados = {}
    erros = set()
    with open(caminho, 'r', encoding='utf-8') as f:
        conteudo = f.read()
    blocos = conteudo.strip().split('----------------------------------------')
    for bloco in blocos:
        bloco = bloco.strip()
        if not bloco:
            continue
        erro_match = re.search(r'Erro: Tempo limite excedido ao processar (\S+)', bloco)
        if erro_match:
            nome = erro_match.group(1).strip()
            erros.add(nome)
            continue
        nome_match = re.search(r'Arquivo:\s*(\S+)', bloco)
        solucao_match = re.search(r'Solucao:\s*(\d+)', bloco)
        if nome_match and solucao_match:
            nome = nome_match.group(1).strip()
            solucao = int(solucao_match.group(1))
            resultados[nome] = solucao
    return resultados, erros

def parse_segundo_arquivo(caminho):
    resultados = {}
    with open(caminho, 'r', encoding='utf-8') as f:
        conteudo = f.read()
    blocos = conteudo.strip().split('\n\n')
    for bloco in blocos:
        nome_match = re.search(r'instances/(\S+)', bloco)
        bins_match = re.search(r'Bins usados:\s*(\d+)', bloco)
        if nome_match and bins_match:
            nome = nome_match.group(1).strip()
            bins = int(bins_match.group(1))
            resultados[nome] = bins
    return resultados

def comparar_resultados(primeiro_arquivo, segundo_arquivo, arquivo_saida):
    resultados1, erros = parse_primeiro_arquivo(primeiro_arquivo)
    resultados2 = parse_segundo_arquivo(segundo_arquivo)
    with open(arquivo_saida, 'w', encoding='utf-8') as out:
        for nome in sorted(set(list(resultados1.keys()) + list(erros))):
            if nome in erros:
                out.write(f'{nome}: TEMPO EXCEDIDO\n')
            elif nome in resultados1:
                solucao = resultados1[nome]
                bins = resultados2.get(nome)
                if bins is None:
                    out.write(f'{nome}: NÃO ENCONTRADO no segundo arquivo\n')
                elif solucao == bins:
                    out.write(f'{nome}: OK (Solução = {solucao}, Bins usados = {bins})\n')
                else:
                    out.write(f'{nome}: DIFERENTE (Solução = {solucao}, Bins usados = {bins})\n')
            else:
                out.write(f'{nome}: NÃO ENCONTRADO no primeiro arquivo\n')

# Exemplo de uso:
comparar_resultados('resultados_minknap.txt', 'todas', 'relatorio_comparacao_minknap.txt')
