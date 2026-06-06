// ============================================================
// nth.js - Tab 4: Nth Prime
// ============================================================
'use strict';

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

    $('#nth-n')?.addEventListener('keydown', e => {
        if (e.key === 'Enter') $('#nth-find')?.click();
    });
}