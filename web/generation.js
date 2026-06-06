// ============================================================
// generation.js - Tab 2: Prime Generation
// ============================================================
'use strict';

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
        $(`#${id}`)?.addEventListener('keydown', e => {
            if (e.key === 'Enter') $('#gen-generate')?.click();
        });
    });

    $('#gen-copy')?.addEventListener('click', () => {
        const primes = listEl.dataset.primes;
        if (!primes) return;
        navigator.clipboard.writeText(primes).then(() => {
            $('#gen-copy').textContent = I18n.t('genCopied');
            setTimeout(() => { I18n.apply(); }, 1500);
        });
    });
}