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

// 128-bit unsigned integer maximum (2^128 - 1)
const MAX_UINT128 = BigInt('340282366920938463463374607431768211455');

// ---- Safe BigInt conversion ----
function safeBigInt(str) {
    if (!str || str.trim() === '') return null;
    
    const s = str.trim();
    
    // Remove leading zeros
    const trimmed = s.replace(/^0+/, '') || '0';
    
    // Check for scientific notation
    if (/^[\d.]+[eE][+-]?\d+$/.test(trimmed)) {
        return null; // Cannot handle scientific notation safely
    }
    
    // Check for decimal point
    if (trimmed.includes('.')) {
        return null; // Only integers allowed
    }
    
    // Check length (max 39 digits for 128-bit)
    if (trimmed.length > 39) {
        return null;
    }
    
    try {
        const result = BigInt(trimmed);
        if (result > MAX_UINT128) {
            return null;
        }
        return result;
    } catch {
        return null;
    }
}

// ---- Number formatting for large numbers ----
function fmt(n) {
    // Handle BigInt directly
    if (typeof n === 'bigint') {
        return n.toLocaleString('en-US');
    }
    
    // For strings that look like numbers, always use BigInt to avoid precision loss
    if (typeof n === 'string') {
        const trimmed = n.trim();
        if (/^\d+$/.test(trimmed)) {
            return BigInt(trimmed).toLocaleString('en-US');
        }
        return trimmed;
    }
    
    // For numbers within safe integer range, format normally
    if (typeof n === 'number' && Number.isSafeInteger(n)) {
        return n.toLocaleString('en-US');
    }
    
    return String(n);
}

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
        const nBig = safeBigInt(n);
        if (nBig === null || nBig < 2n) {
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
                const num = fmt(n);
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
            const nBig = safeBigInt(n);
            if (nBig === null || nBig < 1n) {
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
                            <span class="result-value prime">${fmt(data.result)}</span>
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
    });
}

// ============================================================
// Tab 2: Prime Generation
// ============================================================
function initPrimeGeneration() {
    const listEl = $('#gen-result-list');

    function renderPrimes(from, to, primes) {
        hideLoading('gen-loading');
        showResult('gen-result');
        $('#gen-result-summary').textContent =
            `${I18n.t('genFound')} ${fmt(primes.length)} ${I18n.t('genPrimes')} [${fmt(from)}, ${fmt(to)}]`;
        listEl.innerHTML = primes.map(p =>
            `<span class="prime-chip">${fmt(p)}</span>`
        ).join('');
        // For copy, format primes with commas
        listEl.dataset.primes = primes.map(p => fmt(p)).join(', ');
    }

    function renderCount(from, to, count) {
        hideLoading('gen-loading');
        showResult('gen-result');
        $('#gen-result-summary').textContent =
            `${I18n.t('genCount')} ${fmt(count)} ${I18n.t('genPrimesIn')} [${fmt(from)}, ${fmt(to)}]`;
        listEl.innerHTML = '';
    }

    $('#gen-generate')?.addEventListener('click', () => {
        const from = val('gen-from');
        const to   = val('gen-to');
        const fromBig = safeBigInt(from);
        const toBig   = safeBigInt(to);
        if (fromBig === null || toBig === null) {
            alert(I18n.t('genInvalidInput')); return;
        }
        if (fromBig > toBig) {
            alert(I18n.t('genInvalidRange')); return;
        }
        if (toBig - fromBig > 10000000n) {
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
        const fromBig = safeBigInt(from);
        const toBig   = safeBigInt(to);
        if (fromBig === null || toBig === null) {
            alert(I18n.t('genInvalidInput')); return;
        }
        if (fromBig > toBig) {
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
        const nBig = safeBigInt(n);
        if (nBig === null || nBig < 2n) {
            alert(I18n.t('facInvalid')); return;
        }
        hideResult('fac-result');
        showLoading('fac-loading');
        apiGet('/api/factorize', { n })
            .then(data => {
                hideLoading('fac-loading');
                showResult('fac-result');
                const factors = data.result;
                let productCheck = 1n;
                let html = '';
                factors.forEach(([prime, exp]) => {
                    const bigP = BigInt(prime);
                    for (let e = 0; e < exp; e++) productCheck *= bigP;
                    html += `
                        <div class="factor-item">
                            <span class="factor-prime">${fmt(prime)}</span>
                            ${exp > 1 ? `<span class="factor-exp">^${exp}</span>` : ''}
                        </div>`;
                });
                const valid = productCheck === nBig;
                html += `
                    <div class="factor-validation ${valid ? 'valid' : ''}">
                        ${I18n.t('facProductCheck')} ${fmt(productCheck.toString())}
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
        const nBig = safeBigInt(n);
        if (nBig === null || nBig < 1n || nBig > 10000000n) {
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
                            ${I18n.t('nthThe')} ${fmt(n)}${I18n.t('nthth')} ${fmt(data.result)}
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