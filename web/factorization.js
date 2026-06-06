// ============================================================
// factorization.js - Tab 3: Factorization
// ============================================================
'use strict';

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

    $('#fac-n')?.addEventListener('keydown', e => {
        if (e.key === 'Enter') $('#fac-factorize')?.click();
    });
}