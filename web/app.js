// ============================================================
// app.js - Client-side controller
// Responsibilities:
//   1. Settings dropdown (i18n trigger)
//   2. Tab navigation
//   3. API call orchestration (primality, generation, factorization, nth-prime)
//   4. DOM result rendering
// All translation strings are delegated to I18n.t().
// ============================================================

'use strict';

// ---- Singleton selectors ----
const $ = (s, c = document) => c.querySelector(s);
const $$ = (s, c = document) => [...c.querySelectorAll(s)];

// ---- API Client ----
async function apiGet(endpoint, params = {}) {
    const url = new URL(endpoint, location.origin);
    Object.entries(params).forEach(([k, v]) => url.searchParams.set(k, v));
    const resp = await fetch(url.toString());
    if (!resp.ok) {
        const err = await resp.json().catch(() => ({ error: `HTTP ${resp.status}` }));
        throw new Error(err.error || `HTTP ${resp.status}`);
    }
    return resp.json();
}

// ---- Number formatting ----
const fmt = n => Number(n).toLocaleString('en-US');

// ---- UI helpers ----
const showLoading = id => $(`#${id}`)?.classList.remove('hidden');
const hideLoading = id => $(`#${id}`)?.classList.add('hidden');
const showResult  = id => $(`#${id}`)?.classList.remove('hidden');
const hideResult  = id => $(`#${id}`)?.classList.add('hidden');
const val = id => $(`#${id}`)?.value.trim() ?? '';

// ---- Animation re-trigger for tab panels ----
const triggerReveal = el => {
    el.style.animation = 'none';
    el.offsetHeight;
    el.style.animation = '';
};

// ============================================================
// Settings & I18n
// ============================================================
function initSettings() {
    const btn     = $('#settings-btn');
    const dropdown = $('#settings-dropdown');

    btn.addEventListener('click', e => {
        e.stopPropagation();
        const isOpen = dropdown.classList.toggle('hidden');
        btn.classList.toggle('open', !isOpen);
    });

    document.addEventListener('click', e => {
        if (!dropdown.contains(e.target) && !btn.contains(e.target)) {
            dropdown.classList.add('hidden');
            btn.classList.remove('open');
        }
    });

    $$('.lang-option').forEach(opt => {
        opt.addEventListener('click', () => {
            I18n.setLang(opt.dataset.lang);
            dropdown.classList.add('hidden');
            btn.classList.remove('open');
        });
    });
}

// ============================================================
// Tab Navigation
// ============================================================
function initTabs() {
    $$('.tab-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            const target = btn.dataset.tab;

            $$('.tab-btn').forEach(b => b.classList.remove('active'));
            btn.classList.add('active');

            $$('.tab-panel').forEach(panel => {
                const match = panel.id === `tab-${target}`;
                panel.classList.toggle('active', match);
                if (match) triggerReveal(panel);
            });
        });
    });
}

// ============================================================
// Tab 1: Primality Test
// ============================================================
function initPrimalityTest() {
    $('#pt-check')?.addEventListener('click', () => {
        const n = val('pt-n');
        if (!n || isNaN(n) || BigInt(n) < 2n) {
            alert(I18n.t('ptInvalid'));
            return;
        }
        hideResult('pt-result');
        showLoading('pt-loading');
        apiGet('/api/is_prime', { n })
            .then(data => {
                hideLoading('pt-loading');
                showResult('pt-result');
                const isPrime = data.result;
                const num = fmt(Number(n));
                $('#pt-result').innerHTML = `
                    <div class="result-card">
                        <span class="result-label">${I18n.t('ptResult')}</span>
                        <span class="result-value ${isPrime ? 'prime' : 'composite'}">
                            ${num} ${isPrime ? I18n.t('ptIsPrime') : I18n.t('ptIsComposite')}
                        </span>
                    </div>`;
            })
            .catch(err => {
                hideLoading('pt-loading');
                showResult('pt-result');
                $('#pt-result').innerHTML = `
                    <div class="result-card">
                        <span class="result-label">${I18n.t('ptError')}</span>
                        <span class="result-value composite">${err.message}</span>
                    </div>`;
            });
    });

    // Enter key
    $('#pt-n')?.addEventListener('keydown', e => { if (e.key === 'Enter') $('#pt-check')?.click(); });

    // Quick navigation
    $$('.pt-quick').forEach(btn => {
        btn.addEventListener('click', () => {
            const n = val('pt-n');
            if (!n || isNaN(n) || BigInt(n) < 1n) {
                alert(I18n.t('ptInvalid'));
                return;
            }
            hideResult('pt-result');
            showLoading('pt-loading');
            const endpoint = btn.dataset.action === 'next' ? 'next_prime' : 'prev_prime';
            const label   = btn.dataset.action === 'next' ? I18n.t('ptNextLabel') : I18n.t('ptPrevLabel');
            apiGet(`/api/${endpoint}`, { n })
                .then(data => {
                    hideLoading('pt-loading');
                    showResult('pt-result');
                    $('#pt-result').innerHTML = `
                        <div class="result-card">
                            <span class="result-label">${label}</span>
                            <span class="result-value prime">${fmt(Number(data.result))}</span>
                        </div>`;
                    $('#pt-n').value = data.result;
                })
                .catch(err => {
                    hideLoading('pt-loading');
                    showResult('pt-result');
                    $('#pt-result').innerHTML = `
                        <div class="result-card">
                            <span class="result-label">${I18n.t('ptError')}</span>
                            <span class="result-value composite">${err.message}</span>
                        </div>`;
                });
        });
    });
}

// ============================================================
// Tab 2: Prime Generation
// ============================================================
function initPrimeGeneration() {
    const listEl = $('#gen-result-list');

    function renderPrimes(from, to, primes) {
        showResult('gen-result');
        $('#gen-result-summary').textContent =
            `${I18n.t('genFound')} ${fmt(primes.length)} ${I18n.t('genPrimes')} [${fmt(Number(from))}, ${fmt(Number(to))}]`;
        listEl.innerHTML = primes.map(p =>
            `<span class="prime-chip">${fmt(Number(p))}</span>`
        ).join('');
        listEl.dataset.primes = primes.join(', ');
    }

    function renderCount(from, to, count) {
        showResult('gen-result');
        $('#gen-result-summary').textContent =
            `${I18n.t('genCount')} ${fmt(count)} ${I18n.t('genPrimesIn')} [${fmt(Number(from))}, ${fmt(Number(to))}]`;
        listEl.innerHTML = '';
    }

    $('#gen-generate')?.addEventListener('click', () => {
        const from = val('gen-from');
        const to   = val('gen-to');
        if (!from || !to || isNaN(from) || isNaN(to)) {
            alert(I18n.t('genInvalidInput')); return;
        }
        if (BigInt(from) > BigInt(to)) {
            alert(I18n.t('genInvalidRange')); return;
        }
        if (BigInt(to) - BigInt(from) > 10000000n) {
            alert(I18n.t('genRangeTooLarge')); return;
        }
        hideResult('gen-result');
        showLoading('gen-loading');
        apiGet('/api/primes', { from, to })
            .then(data => renderPrimes(from, to, data.result))
            .catch(err => {
                hideLoading('gen-loading');
                showResult('gen-result');
                $('#gen-result-summary').textContent = `${I18n.t('genError')} ${err.message}`;
            });
    });

    $('#gen-count')?.addEventListener('click', () => {
        const from = val('gen-from');
        const to   = val('gen-to');
        if (!from || !to || isNaN(from) || isNaN(to)) {
            alert(I18n.t('genInvalidInput')); return;
        }
        if (BigInt(from) > BigInt(to)) {
            alert(I18n.t('genInvalidRange')); return;
        }
        hideResult('gen-result');
        showLoading('gen-loading');
        apiGet('/api/primes_count', { from, to })
            .then(data => renderCount(from, to, data.result))
            .catch(err => {
                hideLoading('gen-loading');
                showResult('gen-result');
                $('#gen-result-summary').textContent = `${I18n.t('genError')} ${err.message}`;
            });
    });

    ['gen-from', 'gen-to'].forEach(id => {
        $(`#${id}`)?.addEventListener('keydown', e => { if (e.key === 'Enter') $('#gen-generate')?.click(); });
    });

    $('#gen-copy')?.addEventListener('click', () => {
        const primes = listEl.dataset.primes;
        if (!primes) return;
        navigator.clipboard.writeText(primes).then(() => {
            $('#gen-copy').textContent = I18n.t('genCopied');
            setTimeout(() => {
                I18n.apply();
            }, 1500);
        });
    });
}

// ============================================================
// Tab 3: Factorization
// ============================================================
function initFactorization() {
    $('#fac-factorize')?.addEventListener('click', () => {
        const n = val('fac-n');
        if (!n || isNaN(n) || BigInt(n) < 2n) {
            alert(I18n.t('facInvalid')); return;
        }
        hideResult('fac-result');
        showLoading('fac-loading');
        apiGet('/api/factorize', { n })
            .then(data => {
                hideLoading('fac-loading');
                showResult('fac-result');
                const factors = data.result;
                const bigN = BigInt(n);
                let productCheck = 1n;
                let html = '';
                factors.forEach(([prime, exp]) => {
                    const bigP = BigInt(prime);
                    for (let e = 0; e < exp; e++) productCheck *= bigP;
                    html += `
                        <div class="factor-item">
                            <span class="factor-prime">${fmt(Number(prime))}</span>
                            ${exp > 1 ? `<span class="factor-exp">^${exp}</span>` : ''}
                        </div>`;
                });
                const valid = productCheck === bigN;
                html += `
                    <div class="factor-validation ${valid ? 'valid' : ''}">
                        ${I18n.t('facProductCheck')} ${fmt(Number(productCheck))}
                        (${valid ? I18n.t('facMatch') : I18n.t('facMismatch')})
                    </div>`;
                $('#fac-tree').innerHTML = html;
            })
            .catch(err => {
                hideLoading('fac-loading');
                showResult('fac-result');
                $('#fac-tree').innerHTML = `<div class="factor-validation" style="color:var(--error)">${I18n.t('genError')} ${err.message}</div>`;
            });
    });

    $('#fac-n')?.addEventListener('keydown', e => { if (e.key === 'Enter') $('#fac-factorize')?.click(); });
}

// ============================================================
// Tab 4: Nth Prime
// ============================================================
function initNthPrime() {
    $('#nth-find')?.addEventListener('click', () => {
        const n = val('nth-n');
        if (!n || isNaN(n) || BigInt(n) < 1n || BigInt(n) > 10000000n) {
            alert(I18n.t('nthInvalid')); return;
        }
        hideResult('nth-result');
        showLoading('nth-loading');
        apiGet('/api/nth_prime', { n })
            .then(data => {
                hideLoading('nth-loading');
                showResult('nth-result');
                $('#nth-result').innerHTML = `
                    <div class="result-card">
                        <span class="result-label">${I18n.t('nthResult')}</span>
                        <span class="result-value prime">
                            ${I18n.t('nthThe')} ${fmt(Number(n))}${I18n.t('nthth')} ${fmt(Number(data.result))}
                        </span>
                    </div>`;
            })
            .catch(err => {
                hideLoading('nth-loading');
                showResult('nth-result');
                $('#nth-result').innerHTML = `
                    <div class="result-card">
                        <span class="result-label">${I18n.t('ptError')}</span>
                        <span class="result-value composite">${err.message}</span>
                    </div>`;
            });
    });

    $('#nth-n')?.addEventListener('keydown', e => { if (e.key === 'Enter') $('#nth-find')?.click(); });
}

// ============================================================
// Bootstrap
// ============================================================
document.addEventListener('DOMContentLoaded', () => {
    I18n.init();      // Load saved language, apply all data-i18n
    initSettings();   // Settings dropdown + language switcher
    initTabs();       // Tab navigation
    initPrimalityTest();
    initPrimeGeneration();
    initFactorization();
    initNthPrime();
});