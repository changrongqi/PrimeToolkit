// ============================================================
// app.js - Common utilities and bootstrap
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
    const trimmed = s.replace(/^0+/, '') || '0';
    if (/^[\d.]+[eE][+-]?\d+$/.test(trimmed)) return null;
    if (trimmed.includes('.')) return null;
    if (trimmed.length > 39) return null;
    try {
        const result = BigInt(trimmed);
        if (result > MAX_UINT128) return null;
        return result;
    } catch {
        return null;
    }
}

// ---- Number formatting for large numbers ----
function fmt(n) {
    if (typeof n === 'bigint') {
        return n.toLocaleString('en-US');
    }
    if (typeof n === 'string') {
        const trimmed = n.trim();
        if (/^\d+$/.test(trimmed)) {
            return BigInt(trimmed).toLocaleString('en-US');
        }
        return trimmed;
    }
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
// Bootstrap
// ============================================================
document.addEventListener('DOMContentLoaded', () => {
    I18n.init();
    initSettings();
    initTabs();
    initPrimalityTest();
    initPrimeGeneration();
    initFactorization();
    initNthPrime();
});