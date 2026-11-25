# Snake 360 — LibXenon

Jogo Snake em C++ para Xbox 360 RGH/JTAG, com resolução de superfície 640×480 e visual retrô inspirado no Nokia.

## Recursos
- Menu de título, instruções e tela de Game Over
- Cobra cresce ao comer e ganha velocidade
- Colisão com paredes e com o próprio corpo
- Pontuação e recorde durante a sessão
- D-Pad/analógico e botões A/B/X
- Build automático no GitHub Actions

## Controles
- D-Pad ou analógico esquerdo: mover
- A: iniciar / confirmar / tentar novamente
- B: voltar ao menu na tela de Game Over
- X: instruções no menu
- Start/ESC: sair (ESC é útil em ambientes de teste)

## Compilação
O workflow `.github/workflows/build.yml` tenta compilar usando a imagem `free60/libxenon`.
Após enviar os arquivos ao GitHub:
1. Abra a aba **Actions**.
2. Selecione **Build Snake 360 (LibXenon)**.
3. Clique em **Run workflow** ou faça um push.
4. Baixe o artifact `Snake360-XeLL-ELF`.

## Observações importantes
- O alvo é ELF32 para iniciar pelo XeLL Reloaded, não um XEX de dashboard.
- A saída 640×480 é solicitada por `SDL_SetVideoMode`; a resolução física efetiva depende do modo de vídeo ativo/da implementação SDL-Xenon.
- Este primeiro pacote ainda precisa ser compilado no runner e testado no hardware. O container `free60/libxenon` pode não incluir SDL-Xenon; se o linker acusar falta de `SDL/SDL.h` ou `-lSDL`, será necessário adicionar a SDL-Xenon ao ambiente de build. Não considere o ELF validado até o Actions concluir e o console executar o jogo.
- O recorde é mantido apenas durante a execução; ainda não há gravação em USB.

## Referências
- LibXenon: https://github.com/X360Tools/libxenon
- Projeto SDL para LibXenon: https://github.com/LibXenonProject/libSDLXenon
