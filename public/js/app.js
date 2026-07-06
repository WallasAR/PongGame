// Função para atualizar o placar e UI vindo do C++
        let lastScore1 = 0;
        let lastScore2 = 0;
        let isBallVisible = true;
        let currentDx = 0;
        let currentDy = 0;
        
        let lastDrawWidth = -1; // To avoid regenerating HTML every frame
        
        window.updateUI = function(score1, score2, isMenu, isPaused, bX, bY, showBall, dx, dy, spec1, spec2, timerFrames, deadZoneWidth) {
            if (score1 !== lastScore1) {
                document.getElementById('score1').innerText = score1;
                lastScore1 = score1;
            }
            if (score2 !== lastScore2) {
                document.getElementById('score2').innerText = score2;
                lastScore2 = score2;
            }
            
            let isSettingsActive = document.getElementById('settings-menu-view').style.display === 'flex';
            document.getElementById('menu-overlay').style.display = (isMenu || isSettingsActive) ? 'flex' : 'none';
            document.getElementById('pause-overlay').style.display = (isPaused && !isMenu && !isSettingsActive) ? 'flex' : 'none';
            
            document.getElementById('special1').style.width = spec1 + '%';
            document.getElementById('special2').style.width = spec2 + '%';
            
            const timerEl = document.getElementById('game-timer');
            const skullL = document.getElementById('skull-left');
            const skullR = document.getElementById('skull-right');
            const skullInnerL = document.getElementById('skull-inner-l');
            const skullInnerR = document.getElementById('skull-inner-r');
            
            if (timerFrames >= 0 && !isMenu) {
                timerEl.style.display = 'block';
                let totalSeconds = Math.floor(timerFrames / 60);
                let m = Math.floor(totalSeconds / 60);
                let s = totalSeconds % 60;
                timerEl.innerText = (m < 10 ? "0" + m : m) + ":" + (s < 10 ? "0" + s : s);
                
                // Update Skull Zones
                let timeToShrink = timerFrames % 1200;
                let isWarning = (timeToShrink <= 180 && timerFrames > 0);
                let currentDrawWidth = deadZoneWidth;
                
                if (isWarning) {
                    currentDrawWidth += 60; // Show warning over the next shrinking area
                }
                
                if (currentDrawWidth > 0) {
                    skullL.style.display = 'block';
                    skullR.style.display = 'block';
                    skullL.style.width = (currentDrawWidth / 960 * 100) + '%';
                    skullR.style.width = (currentDrawWidth / 960 * 100) + '%';
                    
                    // Generate new columns if width changed
                    if (currentDrawWidth !== lastDrawWidth) {
                        lastDrawWidth = currentDrawWidth;
                        let numCols = Math.max(1, Math.floor(currentDrawWidth / 60));
                        let skullColHtml = '<div class="skull-col">' + '<span class="skull-char">☠</span>'.repeat(8) + '</div>';
                        let allHtml = skullColHtml.repeat(numCols);
                        skullInnerL.innerHTML = allHtml;
                        skullInnerR.innerHTML = allHtml;
                    }
                    
                    // Flash skulls if it's about to shrink
                    if (isWarning && (timerFrames % 30) > 15) {
                        skullInnerL.style.color = '#ff0000'; // Bright red
                        skullInnerR.style.color = '#ff0000';
                        skullInnerL.style.opacity = '1';
                        skullInnerR.style.opacity = '1';
                    } else {
                        skullInnerL.style.color = '#aa0000'; // Dark red
                        skullInnerR.style.color = '#aa0000';
                        skullInnerL.style.opacity = '0.3';
                        skullInnerR.style.opacity = '0.3';
                    }
                } else {
                    skullL.style.display = 'none';
                    skullR.style.display = 'none';
                    lastDrawWidth = 0;
                }
            } else {
                timerEl.style.display = 'none';
                skullL.style.display = 'none';
                skullR.style.display = 'none';
                lastDrawWidth = -1;
            }
            
            currentDx = dx;
            currentDy = dy;
            
            // Atualizar posicao da bola ASCII
            const asciiBall = document.getElementById('ascii-ball');
            if (isMenu || !showBall) {
                asciiBall.style.display = 'none';
                for (let i=0; i<5; i++) document.getElementById('ascii-trail-'+i).style.display = 'none';
                window.trailHistory = [];
            } else {
                asciiBall.style.display = 'block';
                const leftPos = (bX / 960 * 100) + "%";
                const bottomPos = (bY / 500 * 100) + "%";
                asciiBall.style.left = leftPos;
                asciiBall.style.bottom = bottomPos;
                
                // Trails
                if (!window.trailHistory) window.trailHistory = [];
                let th = window.trailHistory;
                
                if (th.length > 0) {
                    let lastP = th[0];
                    if (Math.abs(bX - lastP.x) > 5 || Math.abs(bY - lastP.y) > 5) {
                        th.unshift({left: leftPos, bottom: bottomPos, x: bX, y: bY, text: asciiBall.innerText});
                    }
                } else {
                    th.unshift({left: leftPos, bottom: bottomPos, x: bX, y: bY, text: asciiBall.innerText});
                }
                
                if (th.length > 5) th.pop();
                
                for (let i=0; i<5; i++) {
                    let tEl = document.getElementById('ascii-trail-'+i);
                    if (i < th.length) {
                        tEl.style.display = 'block';
                        tEl.style.left = th[i].left;
                        tEl.style.bottom = th[i].bottom;
                        tEl.innerText = th[i].text;
                        tEl.style.opacity = (5 - i) / 7.0; 
                        tEl.style.transform = `translate(-50%, 50%) scale(${0.4 - (i * 0.05)})`;
                    } else {
                        tEl.style.display = 'none';
                    }
                }
            }
        };

        let selectedRules = 0; // 0=CLASSIC, 1=DEAD_ZONE

        function showMainMenu() {
            document.getElementById('main-menu-view').style.display = 'flex';
            document.getElementById('mode-menu-view').style.display = 'none';
            document.getElementById('opponent-menu-view').style.display = 'none';
        }
        function showModeMenu() {
            document.getElementById('main-menu-view').style.display = 'none';
            document.getElementById('mode-menu-view').style.display = 'flex';
            document.getElementById('opponent-menu-view').style.display = 'none';
            document.getElementById('difficulty-menu-view').style.display = 'none';
            document.getElementById('loadout-menu-view').style.display = 'none';
        }
        function selectMode(rules) {
            selectedRules = rules;
            document.getElementById('main-menu-view').style.display = 'none';
            document.getElementById('mode-menu-view').style.display = 'none';
            document.getElementById('opponent-menu-view').style.display = 'flex';
        }

        function showDifficultyMenu() {
            document.getElementById('opponent-menu-view').style.display = 'none';
            document.getElementById('difficulty-menu-view').style.display = 'flex';
        }
        function showOpponentMenu() {
            document.getElementById('difficulty-menu-view').style.display = 'none';
            document.getElementById('opponent-menu-view').style.display = 'flex';
        }

        let selectedOpponent = 1;
        let isFromDifficulty = false;
        function showLoadoutMenu(opponent) {
            selectedOpponent = opponent;
            isFromDifficulty = opponent > 1;
            document.getElementById('opponent-menu-view').style.display = 'none';
            document.getElementById('difficulty-menu-view').style.display = 'none';
            document.getElementById('loadout-menu-view').style.display = 'flex';
            
            if (opponent === 1) {
                document.getElementById('p2-loadout-block').style.display = 'block';
            } else {
                document.getElementById('p2-loadout-block').style.display = 'none';
            }
        }
        function goBackToOpponentMenu() {
            document.getElementById('loadout-menu-view').style.display = 'none';
            if (isFromDifficulty) {
                document.getElementById('difficulty-menu-view').style.display = 'flex';
            } else {
                document.getElementById('opponent-menu-view').style.display = 'flex';
            }
        }
        function startGameActual() {
            if (Module && Module._set_loadout) {
                const p1f = parseInt(document.getElementById('p1-fwd').value);
                const p1b = parseInt(document.getElementById('p1-bwd').value);
                const p1n = parseInt(document.getElementById('p1-neu').value);
                Module._set_loadout(1, p1f, p1b, p1n);
                
                if (selectedOpponent === 1) {
                    const p2f = parseInt(document.getElementById('p2-fwd').value);
                    const p2b = parseInt(document.getElementById('p2-bwd').value);
                    const p2n = parseInt(document.getElementById('p2-neu').value);
                    Module._set_loadout(2, p2f, p2b, p2n);
                }
            }
            if (Module && Module._start_game) {
                Module._start_game(selectedRules, selectedOpponent);
            }
            document.getElementById('loadout-menu-view').style.display = 'none';
        }
        
        function resumeGame() {
            if (Module && Module._trigger_action) Module._trigger_action(0);
        }
        function restartGame() {
            if (Module && Module._trigger_action) Module._trigger_action(1);
        }
        function quitToMenu() {
            if (Module && Module._trigger_action) Module._trigger_action(2);
            showMainMenu(); // Reset the UI state
        }

        let isSettingsFromPause = false;
        
        function showSettingsMenu() {
            isSettingsFromPause = document.getElementById('pause-overlay').style.display === 'flex';
            document.getElementById('main-menu-view').style.display = 'none';
            document.getElementById('mode-menu-view').style.display = 'none';
            document.getElementById('opponent-menu-view').style.display = 'none';
            document.getElementById('pause-overlay').style.display = 'none';
            
            document.getElementById('menu-overlay').style.display = 'flex';
            document.getElementById('settings-menu-view').style.display = 'flex';
        }
        
        function closeSettingsMenu() {
            document.getElementById('settings-menu-view').style.display = 'none';
            if (isSettingsFromPause) {
                document.getElementById('menu-overlay').style.display = 'none';
                document.getElementById('pause-overlay').style.display = 'flex';
            } else {
                document.getElementById('main-menu-view').style.display = 'flex';
            }
        }
        
        const glutSpecials = {
            "ArrowLeft": 100, "ArrowUp": 101, "ArrowRight": 102, "ArrowDown": 103,
            "PageUp": 104, "PageDown": 105, "Home": 106, "End": 107, "Insert": 108
        };
        
        const defaultBinds = [
            { id: 0, text: "UP", keyText: "W", key: 119, isSpecial: false },
            { id: 1, text: "DOWN", keyText: "S", key: 115, isSpecial: false },
            { id: 2, text: "NEUTRAL", keyText: "SPACE", key: 32, isSpecial: false },
            { id: 3, text: "UP", keyText: "ARROW UP", key: 101, isSpecial: true },
            { id: 4, text: "DOWN", keyText: "ARROW DOWN", key: 103, isSpecial: true },
            { id: 5, text: "NEUTRAL", keyText: "NUM 0", key: 48, isSpecial: false },
            { id: 6, text: "PAUSE", keyText: "ENTER", key: 13, isSpecial: false },
            { id: 7, text: "FORWARD", keyText: "D", key: 100, isSpecial: false },
            { id: 8, text: "BACKWARD", keyText: "A", key: 97, isSpecial: false },
            { id: 9, text: "FORWARD", keyText: "ARROW LEFT", key: 100, isSpecial: true },
            { id: 10, text: "BACKWARD", keyText: "ARROW RIGHT", key: 102, isSpecial: true }
        ];
        
        let currentBinds = JSON.parse(localStorage.getItem('pongBinds')) || defaultBinds;
        
        function applyBindsToCpp() {
            if (Module && Module._set_key_bind) {
                currentBinds.forEach(b => {
                    Module._set_key_bind(b.id, b.key, b.isSpecial);
                });
            } else {
                setTimeout(applyBindsToCpp, 500);
            }
        }
        applyBindsToCpp();
        
        function updateBindButtons() {
            const btns = document.querySelectorAll('.bind-btn');
            if (btns.length === 0) return;
            btns.forEach((btn) => {
                const match = btn.getAttribute('onclick').match(/listenKey\((\d+)/);
                if (match) {
                    const actionId = parseInt(match[1]);
                    const bindObj = currentBinds.find(b => b.id === actionId);
                    if(bindObj) {
                        btn.innerText = bindObj.text + ": " + bindObj.keyText;
                    }
                }
            });
        }
        
        document.addEventListener('DOMContentLoaded', updateBindButtons);
        
        const skillsList = [
            { id: 1, name: 'SMASH (Straight Fast)' },
            { id: 2, name: 'SHIELD (Bigger Paddle)' },
            { id: 3, name: 'GHOST (Invisible Ball)' },
            { id: 4, name: 'BLINK (Teleport Ball)' },
            { id: 5, name: 'FREEZE (Slow Opponent)' },
            { id: 6, name: 'EMP (Reverse Controls)' }
        ];
        
        function populateLoadoutDropdowns() {
            const selects = ['p1-fwd', 'p1-bwd', 'p1-neu', 'p2-fwd', 'p2-bwd', 'p2-neu'];
            selects.forEach((id, index) => {
                const el = document.getElementById(id);
                skillsList.forEach(s => {
                    const opt = document.createElement('option');
                    opt.value = s.id;
                    opt.innerText = s.name;
                    el.appendChild(opt);
                });
                el.value = (index % 3) + 1;
            });
        }
        document.addEventListener('DOMContentLoaded', populateLoadoutDropdowns);
        
        let listeningBtn = null;
        let listeningId = -1;
        
        function listenKey(actionId, btnEl) {
            if (listeningBtn) return;
            listeningBtn = btnEl;
            listeningId = actionId;
            btnEl.innerText = "PRESS ANY KEY...";
            
            const handler = function(e) {
                e.preventDefault();
                e.stopPropagation();
                
                let keyName = e.key.toUpperCase();
                if (e.code === 'Space') keyName = 'SPACE';
                
                let keyCode = 0;
                let isSpecial = false;
                
                if (glutSpecials[e.key]) {
                    keyCode = glutSpecials[e.key];
                    isSpecial = true;
                    keyName = e.key.replace(/([A-Z])/g, ' $1').trim().toUpperCase();
                } else if (e.key === 'Enter') {
                    keyCode = 13;
                } else if (e.code === 'Space') {
                    keyCode = 32;
                } else if (e.key.length === 1) {
                    keyCode = e.key.toLowerCase().charCodeAt(0);
                } else {
                    return; // Ignore unknown keys like Shift, Control without another key
                }
                
                currentBinds[actionId].keyText = keyName;
                currentBinds[actionId].key = keyCode;
                currentBinds[actionId].isSpecial = isSpecial;
                
                localStorage.setItem('pongBinds', JSON.stringify(currentBinds));
                
                if (Module && Module._set_key_bind) {
                    Module._set_key_bind(actionId, keyCode, isSpecial);
                }
                
                updateBindButtons();
                listeningBtn = null;
                listeningId = -1;
                
                window.removeEventListener('keydown', handler, true);
            };
            
            window.addEventListener('keydown', handler, true);
        }

        function createDomExplosion(x, y) {
            const numParticles = 15;
            const chars = ['*', '+', 'x', '#', '@'];
            for (let i = 0; i < numParticles; i++) {
                const particle = document.createElement('div');
                particle.innerText = chars[Math.floor(Math.random() * chars.length)];
                particle.style.position = 'fixed';
                particle.style.left = x + 'px';
                particle.style.top = y + 'px';
                particle.style.color = 'white';
                particle.style.fontSize = (Math.random() * 0.8 + 0.4) + 'rem';
                particle.style.pointerEvents = 'none';
                particle.style.zIndex = 1000;
                document.body.appendChild(particle);
                
                const angle = Math.random() * Math.PI * 2;
                const speed = Math.random() * 80 + 30;
                const dx = Math.cos(angle) * speed;
                const dy = Math.sin(angle) * speed;
                
                const anim = particle.animate([
                    { transform: 'translate(0, 0)', opacity: 1 },
                    { transform: `translate(${dx}px, ${dy}px)`, opacity: 0 }
                ], {
                    duration: 300 + Math.random() * 300,
                    easing: 'ease-out'
                });
                
                anim.onfinish = () => particle.remove();
            }
        }

        document.addEventListener('click', function(e) {
            if (e.target.tagName === 'BUTTON') {
                createDomExplosion(e.clientX, e.clientY);
            }
        });

        // Funções de Audio via Web Audio API injetadas no emscripten
        const audioCtx = new (window.AudioContext || window.webkitAudioContext)();
        
        window.playBeep = function(freq, durationMs) {
            if (audioCtx.state === 'suspended') {
                audioCtx.resume();
            }
            const oscillator = audioCtx.createOscillator();
            const gainNode = audioCtx.createGain();
            
            oscillator.type = 'sine'; // som estilo retro
            oscillator.frequency.value = freq;
            
            oscillator.connect(gainNode);
            gainNode.connect(audioCtx.destination);
            
            oscillator.start();
            
            // Fade out para não estalar
            gainNode.gain.setValueAtTime(1, audioCtx.currentTime);
            gainNode.gain.exponentialRampToValueAtTime(0.001, audioCtx.currentTime + (durationMs / 1000));
            
            oscillator.stop(audioCtx.currentTime + (durationMs / 1000));
        };

        window.playExplosion = function() {
            if (audioCtx.state === 'suspended') audioCtx.resume();
            const duration = 0.5; // half second explosion
            const bufferSize = audioCtx.sampleRate * duration;
            const buffer = audioCtx.createBuffer(1, bufferSize, audioCtx.sampleRate);
            const data = buffer.getChannelData(0);
            for (let i = 0; i < bufferSize; i++) {
                data[i] = Math.random() * 2 - 1; // White noise
            }
            const noise = audioCtx.createBufferSource();
            noise.buffer = buffer;
            
            const filter = audioCtx.createBiquadFilter();
            filter.type = 'lowpass';
            filter.frequency.value = 1000; // Muffle the noise for a retro explosion
            
            const gainNode = audioCtx.createGain();
            gainNode.gain.setValueAtTime(1, audioCtx.currentTime);
            gainNode.gain.exponentialRampToValueAtTime(0.01, audioCtx.currentTime + duration);
            
            noise.connect(filter);
            filter.connect(gainNode);
            gainNode.connect(audioCtx.destination);
            noise.start();
        };

        // Matrizes e rotacoes para a Fisica 3D da bola
        let ballRotMat = [1,0,0, 0,1,0, 0,0,1];
        function matMult(A, B) {
            return [
                A[0]*B[0]+A[1]*B[3]+A[2]*B[6], A[0]*B[1]+A[1]*B[4]+A[2]*B[7], A[0]*B[2]+A[1]*B[5]+A[2]*B[8],
                A[3]*B[0]+A[4]*B[3]+A[5]*B[6], A[3]*B[1]+A[4]*B[4]+A[5]*B[7], A[3]*B[2]+A[4]*B[5]+A[5]*B[8],
                A[6]*B[0]+A[7]*B[3]+A[8]*B[6], A[6]*B[1]+A[7]*B[4]+A[8]*B[7], A[6]*B[2]+A[7]*B[5]+A[8]*B[8]
            ];
        }
        function axisAngleMat(ax, ay, az, theta) {
            let c = Math.cos(theta), s = Math.sin(theta), t = 1 - c;
            let len = Math.sqrt(ax*ax + ay*ay + az*az);
            if(len < 0.0001) return [1,0,0, 0,1,0, 0,0,1];
            ax /= len; ay /= len; az /= len;
            return [
                t*ax*ax + c,    t*ax*ay - s*az, t*ax*az + s*ay,
                t*ax*ay + s*az, t*ay*ay + c,    t*ay*az - s*ax,
                t*ax*az - s*ay, t*ay*az + s*ax, t*az*az + c
            ];
        }

        // ASCII 3D Sphere Generator
        const asciiChars = " .,-~:;=!*#$@";
        function renderAsciiSphere() {
            const r = 6; // Reduzido pela metade para ficar do tamanho exato da bola original
            const width = 24; 
            const height = 12;
            
            // Fisica da rotacao: Se move para (dx, dy), gira no eixo (-dy, dx)
            let speed = Math.sqrt(currentDx*currentDx + currentDy*currentDy);
            if (speed > 0) {
                // Aumenta a velocidade de rotacao proporcional ao movimento
                let dTheta = speed * 0.15; // Girar mais rapido por causa do tamanho menor
                let R = axisAngleMat(-currentDy, currentDx, 0, dTheta);
                ballRotMat = matMult(R, ballRotMat);
            }
            
            // Fonte de luz fixa na cena (como um holofote de fliperama)
            let Lx = 0.5;
            let Ly = -1.0;
            let Lz = -1.0;
            let L_len = Math.sqrt(Lx*Lx + Ly*Ly + Lz*Lz);
            Lx /= L_len; Ly /= L_len; Lz /= L_len;
            
            // Inversa da matriz ortogonal eh a sua transposta
            let invM = [
                ballRotMat[0], ballRotMat[3], ballRotMat[6],
                ballRotMat[1], ballRotMat[4], ballRotMat[7],
                ballRotMat[2], ballRotMat[5], ballRotMat[8]
            ];
            
            let html = "";
            for (let y = -height/2; y < height/2; y++) {
                for (let x = -width/2; x < width/2; x++) {
                    let adj_x = x * 0.5; // Corrige o fato da fonte ser mais alta que larga
                    let sq = adj_x*adj_x + y*y;
                    if (sq <= r*r) {
                        let z = Math.sqrt(r*r - sq);
                        // Vetor Normal da superficie da esfera na visao da camera
                        let Nx = adj_x / r;
                        let Ny = y / r;
                        let Nz = z / r;
                        
                        // Rotaciona a normal de volta para o espaco local da bola para calcular a textura
                        let Px = invM[0]*Nx + invM[1]*Ny + invM[2]*Nz;
                        let Py = invM[3]*Nx + invM[4]*Ny + invM[5]*Nz;
                        let Pz = invM[6]*Nx + invM[7]*Ny + invM[8]*Nz;
                        
                        // Textura xadrez 3D para dar ilusao incrivel de rotacao
                        let u = Math.atan2(Px, Pz);
                        let v = Math.asin(Py);
                        let checks = (Math.sin(u * 8) * Math.sin(v * 8) > 0) ? 1.0 : 0.3;
                        
                        // Calculo de Iluminacao pura (Dot product N . L) - depende da camera, nao da textura!
                        let lum = -(Nx*Lx + Ny*Ly + Nz*Lz); 
                        if (lum < 0) lum = 0;
                        
                        // Combina a iluminacao do ambiente com a textura local da bola
                        let final_lum = (0.2 + lum * 0.8) * checks;
                        
                        let charIdx = Math.floor(final_lum * (asciiChars.length - 1));
                        if(charIdx >= asciiChars.length) charIdx = asciiChars.length - 1;
                        html += asciiChars[charIdx];
                    } else {
                        html += " ";
                    }
                }
                html += "\n";
            }
            document.getElementById('ascii-ball').innerText = html;
            requestAnimationFrame(renderAsciiSphere);
        }
        renderAsciiSphere();

        // Configuração do Emscripten Module
        var Module = {
            preRun: [],
            postRun: [
                function() {
                    document.getElementById('loading').style.display = 'none';
                    // Foca o canvas automaticamente
                    document.getElementById('canvas').focus();
                }
            ],
            print: (function() {
                return function(text) {
                    if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
                    console.log(text);
                };
            })(),
            canvas: (function() {
                var canvas = document.getElementById('canvas');
                return canvas;
            })(),
            setStatus: function(text) {
                if (!Module.setStatus.last) Module.setStatus.last = { time: Date.now(), text: '' };
                if (text === Module.setStatus.last.text) return;
                var m = text.match(/([^(]+)\((\d+(\.\d+)?)\/(\d+)\)/);
                var now = Date.now();
                if (m && now - Module.setStatus.last.time < 30) return; 
                Module.setStatus.last.time = now;
                Module.setStatus.last.text = text;
                if (m) {
                    text = m[1];
                }
                if(text) {
                    document.getElementById('loading').innerText = text;
                }
            },
            totalDependencies: 0,
            monitorRunDependencies: function(left) {
                this.totalDependencies = Math.max(this.totalDependencies, left);
                Module.setStatus(left ? 'Preparando... (' + (this.totalDependencies-left) + '/' + this.totalDependencies + ')' : 'Iniciando...');
            }
        };
        Module.setStatus('Fazendo download...');
        window.onerror = function(event) {
            Module.setStatus('Exception thrown, see JavaScript console');
            Module.setStatus = function(text) {
                if (text) console.error('[post-exception status] ' + text);
            };
        };