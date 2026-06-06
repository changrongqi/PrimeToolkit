/* ============================================================
   i18n.js - Internationalization (English + Chinese)
   Single responsibility: provide translation lookup and
   manage locale state with localStorage persistence.
   ============================================================ */

const I18n = (function () {
    'use strict';

    const LANG_KEY = 'prime_toolkit_lang';
    const SUPPORTED = ['en', 'zh'];

    // All translation strings. Structure mirrors the page layout.
    const MESSAGES = {
        en: {
            // Header
            appTitle: 'Prime Number Toolkit',
            appSubtitle: 'High-Performance C++ Core / Deterministic Algorithms',

            // Settings
            settings: 'Settings',
            language: 'Language',
            langEn: 'English',
            langZh: '中文',

            // Tabs
            tabPrimality: 'Primality Test',
            tabGeneration: 'Prime Generation',
            tabFactorization: 'Factorization',
            tabNthPrime: 'Nth Prime',

            // Tab 1: Primality
            ptDesc: 'Deterministic Miller-Rabin for all 64-bit integers. Instant result.',
            ptEnterNumber: 'Enter a number:',
            ptPlaceholder: 'e.g. 999999999989',
            ptCheck: 'Check',
            ptQuickNav: 'Quick navigation:',
            ptNextPrime: 'Next Prime',
            ptPrevPrime: 'Previous Prime',
            ptResult: 'Result:',
            ptIsPrime: 'is PRIME',
            ptIsComposite: 'is COMPOSITE',
            ptNextLabel: 'Next',
            ptPrevLabel: 'Previous',
            ptPrimeLabel: 'prime:',
            ptError: 'Error:',
            ptNoPrimeBelow: 'No prime found below given number',

            // Tab 2: Generation
            genDesc: 'Generate all primes within a range using segmented sieve. Max range: 10,000,000.',
            genFrom: 'From:',
            genTo: 'To:',
            genPlaceholderFrom: 'e.g. 1',
            genPlaceholderTo: 'e.g. 1000',
            genGenerate: 'Generate Primes',
            genCountOnly: 'Count Only',
            genCopy: 'Copy All',
            genCopied: 'Copied!',
            genFound: 'Found',
            genPrimes: 'primes in',
            genCount: 'Count:',
            genPrimesIn: 'primes in',
            genError: 'Error:',
            genRangeTooLarge: 'Range too large. Maximum is 10,000,000.',
            genInvalidRange: '"From" must be <= "To"',
            genInvalidInput: 'Please enter valid integers',

            // Tab 3: Factorization
            facDesc: 'Decompose any 64-bit integer into prime factors. Uses trial division + Pollard\'s Rho.',
            facEnterNumber: 'Enter a number:',
            facPlaceholder: 'e.g. 1234567890123',
            facFactorize: 'Factorize',
            facProductCheck: 'Product check:',
            facMatch: 'MATCH',
            facMismatch: 'MISMATCH',
            facComputationError: 'computation error',
            facInvalid: 'Please enter a valid integer >= 2',

            // Tab 4: Nth Prime
            nthDesc: 'Find the n-th prime number (1-indexed). Supports up to n = 10,000,000.',
            nthEnterN: 'n =',
            nthPlaceholder: 'e.g. 1000000',
            nthFind: 'Find',
            nthResult: 'Result:',
            nthThe: 'The',
            nthth: 'th prime is:',
            nthInvalid: 'Please enter a valid integer between 1 and 10,000,000',

            // Footer
            footer: 'C++ Core · Miller-Rabin · Segmented Sieve · Pollard\'s Rho',

            // Shared
            loading: 'Computing...',
            sieving: 'Sieving range...',
            factoring: 'Factoring...',
            searching: 'Searching...',
            invalidNumber: 'Please enter a valid integer',
            missingParam: 'Missing or invalid parameter',
        },

        zh: {
            // Header
            appTitle: '质数工具箱',
            appSubtitle: '高性能 C++ 内核 / 确定性算法',

            // Settings
            settings: '设置',
            language: '语言',
            langEn: 'English',
            langZh: '中文',

            // Tabs
            tabPrimality: '素性判定',
            tabGeneration: '质数生成',
            tabFactorization: '质因数分解',
            tabNthPrime: '第 N 个质数',

            // Tab 1: Primality
            ptDesc: '全 64 位整数确定性 Miller-Rabin 判定，即时返回结果。',
            ptEnterNumber: '输入数字:',
            ptPlaceholder: '例如: 999999999989',
            ptCheck: '检测',
            ptQuickNav: '快速定位:',
            ptNextPrime: '下一个质数',
            ptPrevPrime: '上一个质数',
            ptResult: '结果:',
            ptIsPrime: '是质数',
            ptIsComposite: '是合数',
            ptNextLabel: '下一个质数:',
            ptPrevLabel: '上一个质数:',
            ptPrimeLabel: 'prime:',
            ptError: '错误:',
            ptNoPrimeBelow: '该数以下没有质数',

            // Tab 2: Generation
            genDesc: '使用分段筛法生成指定范围内的所有质数。最大范围: 10,000,000。',
            genFrom: '起始值:',
            genTo: '终止值:',
            genPlaceholderFrom: '例如: 1',
            genPlaceholderTo: '例如: 1000',
            genGenerate: '生成质数',
            genCountOnly: '仅计数',
            genCopy: '复制全部',
            genCopied: '已复制!',
            genFound: '找到',
            genPrimes: '个质数，区间为',
            genCount: '计数:',
            genPrimesIn: '个质数，区间为',
            genError: '错误:',
            genRangeTooLarge: '范围过大，最大为 10,000,000。',
            genInvalidRange: '"起始值" 必须 <= "终止值"',
            genInvalidInput: '请输入有效整数',

            // Tab 3: Factorization
            facDesc: '将任意 64 位整数分解为质因数。使用试除法 + Pollard\'s Rho 算法。',
            facEnterNumber: '输入数字:',
            facPlaceholder: '例如: 1234567890123',
            facFactorize: '分解质因数',
            facProductCheck: '乘积验证:',
            facMatch: '正确',
            facMismatch: '错误',
            facComputationError: '计算错误',
            facInvalid: '请输入 >= 2 的有效整数',

            // Tab 4: Nth Prime
            nthDesc: '查找第 n 个质数 (从 1 开始计数)。支持 n 最多为 10,000,000。',
            nthEnterN: 'n =',
            nthPlaceholder: '例如: 1000000',
            nthFind: '查找',
            nthResult: '结果:',
            nthThe: '第',
            nthth: '个质数是:',
            nthInvalid: '请输入 1 到 10,000,000 之间的有效整数',

            // Footer
            footer: 'C++ 内核 · Miller-Rabin · 分段筛法 · Pollard\'s Rho',

            // Shared
            loading: '计算中...',
            sieving: '筛法运行中...',
            factoring: '分解中...',
            searching: '搜索中...',
            invalidNumber: '请输入有效整数',
            missingParam: '缺少或无效的参数',
        }
    };

    let currentLang = 'en';

    // ---------- Public API ----------

    function init() {
        currentLang = SUPPORTED.includes(localStorage.getItem(LANG_KEY))
            ? localStorage.getItem(LANG_KEY)
            : 'en';
        apply();
    }

    function setLang(lang) {
        if (!SUPPORTED.includes(lang)) return;
        currentLang = lang;
        localStorage.setItem(LANG_KEY, lang);
        apply();
    }

    function getLang() {
        return currentLang;
    }

    function t(key) {
        return MESSAGES[currentLang][key] ?? MESSAGES['en'][key] ?? key;
    }

    // Apply translations to all [data-i18n] elements
    function apply() {
        $$('[data-i18n]').forEach(el => {
            const key = el.getAttribute('data-i18n');
            const type = el.getAttribute('data-i18n-type');
            if (type === 'placeholder') {
                el.placeholder = t(key);
            } else if (type === 'title') {
                el.title = t(key);
            } else {
                el.textContent = t(key);
            }
        });
        $$('[data-i18n-attr]').forEach(el => {
            const spec = el.getAttribute('data-i18n-attr');
            const [attr, key] = spec.split(':');
            el.setAttribute(attr, t(key));
        });
        // Update active language indicator
        $$('.lang-option').forEach(opt => {
            opt.classList.toggle('active', opt.dataset.lang === currentLang);
        });
        $$('.lang-current').forEach(el => {
            el.textContent = currentLang === 'zh' ? '中文' : 'EN';
        });
    }

    return { init, setLang, getLang, t, apply };

})();