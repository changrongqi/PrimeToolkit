// ============================================================
// primality.js - Tab 1: Primality Test
// ============================================================
'use strict';

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

    $('#pt-n')?.addEventListener('keydown', e => {
        if (e.key === 'Enter') $('#pt-check')?.click();
    });

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