#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>

// --- Configurações do AP ---
const char* ssid = "Wifi_2.4GHZ";   // nome que vai aparecer na lista de Wi-Fi
const char* password = "";          // vazio = rede aberta (sem senha)

DNSServer dnsServer;
WebServer server(80);               // servidor web na porta 80 (padrão HTTP)

const byte DNS_PORT = 53;           // porta padrão do protocolo DNS
IPAddress apIP(192, 168, 4, 1);     // IP que o ESP32 vai ter como AP

// --- Página HTML que vai ser exibida ---
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Sinal capturado</title>
<style>
  :root {
    --bg: #08070d;
    --surface: #100e1a;
    --violet-deep: #3b1d78;
    --violet: #7c4dff;
    --violet-light: #b9a2ff;
    --text: #e9e6f5;
    --muted: #7a768f;
    --line: #221f33;
  }

  * { margin: 0; padding: 0; box-sizing: border-box; }

  html { scroll-behavior: smooth; }

  body {
    background: var(--bg);
    color: var(--text);
    font-family: -apple-system, "Segoe UI", Roboto, sans-serif;
    line-height: 1.6;
    overflow-x: hidden;
  }

  .mono {
    font-family: ui-monospace, "SF Mono", Consolas, monospace;
  }

  /* ---------- HERO ---------- */

  .hero {
    position: relative;
    min-height: 100svh;
    display: flex;
    flex-direction: column;
    justify-content: center;
    padding: 8vh 6vw;
  }

  #signal {
    position: absolute;
    inset: 0;
    width: 100%;
    height: 100%;
    opacity: 0.9;
  }

  .hero-content {
    position: relative;
    z-index: 2;
    max-width: 640px;
  }

  .tag {
    font-size: 0.85rem;
    color: var(--violet-light);
    margin-bottom: 1.5rem;
    display: flex;
    align-items: center;
    gap: 0.6rem;
  }

  .tag::before {
    content: "";
    width: 7px;
    height: 7px;
    border-radius: 50%;
    background: var(--violet-light);
    box-shadow: 0 0 8px var(--violet-light);
    animation: pulse 2s ease-in-out infinite;
  }

  @keyframes pulse {
    0%, 100% { opacity: 1; transform: scale(1); }
    50% { opacity: 0.4; transform: scale(0.7); }
  }

  h1 {
    font-size: clamp(2.2rem, 6vw, 3.8rem);
    font-weight: 600;
    letter-spacing: -0.02em;
    line-height: 1.1;
    max-width: 11ch;
  }

  .hero p.lead {
    margin-top: 1.5rem;
    font-size: 1.1rem;
    color: var(--muted);
    max-width: 46ch;
  }

  .scroll-cue {
    position: absolute;
    bottom: 6vh;
    left: 6vw;
    font-size: 0.8rem;
    color: var(--muted);
    z-index: 2;
  }

  /* ---------- SECTIONS ---------- */

  .track {
    position: relative;
    max-width: 680px;
    margin: 0 auto;
    padding: 0 6vw 12vh;
  }

  section {
    position: relative;
    padding: 5vh 0 5vh 2.2rem;
    border-left: 1px solid var(--line);
  }

  section::before {
    content: "";
    position: absolute;
    left: -4.5px;
    top: 5vh;
    width: 8px;
    height: 8px;
    border-radius: 50%;
    background: var(--violet);
    box-shadow: 0 0 10px var(--violet);
  }

  section h2 {
    font-size: 1.6rem;
    font-weight: 600;
    margin-bottom: 1rem;
    letter-spacing: -0.01em;
  }

  section p {
    color: var(--muted);
    max-width: 54ch;
  }

  section p + p { margin-top: 0.9rem; }

  .steps {
    margin-top: 1.8rem;
    display: flex;
    flex-direction: column;
    gap: 1.4rem;
  }

  .step {
    display: flex;
    gap: 1rem;
    align-items: baseline;
  }

  .step .n {
    font-family: ui-monospace, "SF Mono", Consolas, monospace;
    font-size: 0.85rem;
    color: var(--violet-light);
    min-width: 1.6rem;
  }

  .step .body strong {
    display: block;
    color: var(--text);
    font-weight: 600;
    margin-bottom: 0.2rem;
  }

  .step .body span {
    color: var(--muted);
    font-size: 0.95rem;
  }

  /* ---------- FOOTER ---------- */

  footer {
    padding: 6vh 6vw 8vh;
    border-top: 1px solid var(--line);
    display: flex;
    flex-direction: column;
    gap: 0.5rem;
  }

  footer .mono {
    font-size: 0.85rem;
    color: var(--violet-light);
  }

  footer p {
    color: var(--muted);
    font-size: 0.9rem;
    max-width: 48ch;
  }

  @media (prefers-reduced-motion: reduce) {
    .tag::before { animation: none; }
    html { scroll-behavior: auto; }
  }
</style>
</head>
<body>

<section class="hero">
  <canvas id="signal"></canvas>
  <div class="hero-content">
    <p class="tag mono">conexão local — sem internet</p>
    <h1>Olá.</h1>
    <p class="lead">Esta página não veio da internet. Ela saiu da memória de um ESP32, a poucos metros de você.</p>
  </div>
  <p class="scroll-cue mono">role para entender</p>
</section>

<div class="track">

  <section>
    <h2>O que aconteceu</h2>
    <p>Seu celular procurou uma rede, encontrou esta, e perguntou "tenho internet de verdade aqui?" como faz automaticamente toda vez que você conecta em algo novo.</p>
    <p>A resposta que ele recebeu não veio de um site real. Veio deste pequeno chip, que respondeu no lugar da internet.</p>
  </section>

  <section>
    <h2>Como funciona</h2>
    <p>Três peças simples, rodando ao mesmo tempo:</p>
    <div class="steps">
      <div class="step">
        <span class="n mono">01</span>
        <div class="body">
          <strong>Ponto de acesso</strong>
          <span>O chip cria a própria rede Wi-Fi, em vez de se conectar a uma.</span>
        </div>
      </div>
      <div class="step">
        <span class="n mono">02</span>
        <div class="body">
          <strong>Sequestro de DNS</strong>
          <span>Qualquer endereço que seu celular tenta abrir é respondido com o endereço deste mesmo chip.</span>
        </div>
      </div>
      <div class="step">
        <span class="n mono">03</span>
        <div class="body">
          <strong>Servidor local</strong>
          <span>A página que você está lendo agora foi montada e enviada direto da memória flash dele.</span>
        </div>
      </div>
    </div>
  </section>

  <section>
    <h2>Por que existe</h2>
    <p>Isso é um projeto de estudo sobre redes e segurança usando a mesma técnica usada em portais de Wi-Fi de hotéis e aeroportos, só que construída peça por peça pra entender como funciona por dentro.</p>
    <p>Nada do que você digita ou acessa é salvo. Esta página não pede login, nem coleta nada.</p>
  </section>
   <section>
    <h2>Um cuidado pra levar daqui</h2>
    <p>Esta rede que você acabou de entrar não tinha senha e é exatamente por isso que ela conseguiu te mostrar esta página sem você pedir nada. Uma rede aberta de verdade, com más intenções, poderia fazer o mesmo tipo de coisa sem avisar.</p>
    <div class="steps">
      <div class="step">
        <span class="n mono">•</span>
        <div class="body">
          <strong>Sem senha, sem privacidade</strong>
          <span>Em redes abertas, qualquer pessoa conectada pode ver o tráfego de quem mais está nela.</span>
        </div>
      </div>
      <div class="step">
        <span class="n mono">•</span>
        <div class="body">
          <strong>Redes falsas com tela de login</strong>
          <span>Existe um golpe chamado "evil twin": alguém cria uma rede Wi-Fi aberta com nome parecido com um lugar confiável, e quando você conecta aparece uma tela falsa pedindo login do Facebook ou Instagram pra "liberar a internet". Essa senha nunca chega na rede social de verdade, vai direto pra quem criou a rede.</span>
        </div>
      </div>
      <div class="step">
        <span class="n mono">•</span>
        <div class="body">
          <strong>Evite login e pagamentos</strong>
          <span>Bancos, e-mails e senhas ficam mais seguros esperando uma rede confiável, ou usando os dados do celular.</span>
        </div>
      </div>
    </div>
  </section>

</div>

<footer>
  <p class="mono">Wifi_2.4GHZ // Carbonod8r</p>
  <p>Projeto pessoal de estudo em redes.</p>
</footer>

<script>
  const canvas = document.getElementById('signal');
  const ctx = canvas.getContext('2d');
  let w, h, dpr;

  function resize() {
    dpr = Math.min(window.devicePixelRatio || 1, 2);
    w = canvas.offsetWidth;
    h = canvas.offsetHeight;
    canvas.width = w * dpr;
    canvas.height = h * dpr;
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
  }
  window.addEventListener('resize', resize);
  resize();

  const cx = () => w * 0.78;
  const cy = () => h * 0.42;

  let rings = [];
  let lastSpawn = 0;

  const reduced = window.matchMedia('(prefers-reduced-motion: reduce)').matches;

  function frame(t) {
    ctx.clearRect(0, 0, w, h);

    if (!reduced) {
      if (t - lastSpawn > 1400) {
        rings.push({ r: 0, born: t });
        lastSpawn = t;
      }
    } else if (rings.length === 0) {
      rings.push({ r: 60, born: t });
    }

    rings = rings.filter(ring => {
      const age = reduced ? 0 : t - ring.born;
      const r = reduced ? ring.r : (age / 3200) * Math.max(w, h) * 0.55;
      const alpha = reduced ? 0.35 : Math.max(0, 0.55 - (age / 3200) * 0.55);
      if (alpha <= 0 && !reduced) return false;

      ctx.beginPath();
      ctx.arc(cx(), cy(), r, 0, Math.PI * 2);
      ctx.strokeStyle = `rgba(185, 162, 255, ${alpha})`;
      ctx.lineWidth = 1;
      ctx.stroke();
      return true;
    });

    // núcleo
    const pulse = reduced ? 1 : 0.85 + Math.sin(t / 500) * 0.15;
    const grad = ctx.createRadialGradient(cx(), cy(), 0, cx(), cy(), 90 * pulse);
    grad.addColorStop(0, 'rgba(124, 77, 255, 0.55)');
    grad.addColorStop(1, 'rgba(124, 77, 255, 0)');
    ctx.fillStyle = grad;
    ctx.beginPath();
    ctx.arc(cx(), cy(), 90 * pulse, 0, Math.PI * 2);
    ctx.fill();

    if (!reduced) requestAnimationFrame(frame);
  }
  requestAnimationFrame(frame);
</script>

</body>
</html>
)rawliteral";

// --- Função que envia a página pra quem pedir ---
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

// --- Função "coringa": qualquer URL que não reconhecemos, redireciona pra página principal ---
void handleNotFound() {
  server.sendHeader("Location", "http://192.168.4.1/", true);
  server.send(302, "text/plain", "");
}

void setup() {
  Serial.begin(115200);

  // Cria o Access Point com IP fixo
  WiFi.softAP(ssid, password);
  delay(100);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  // Inicia o DNS Server: responde QUALQUER domínio com o IP do ESP32
  dnsServer.start(DNS_PORT, "*", apIP);

    server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println("Access Point criado!");
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  dnsServer.processNextRequest(); // precisa rodar sempre, senão o DNS trava
  server.handleClient();          // idem pro servidor web
}