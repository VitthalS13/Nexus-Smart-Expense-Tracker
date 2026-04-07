const API_BASE = "http://localhost:8080/api";

const catMap = ["Food", "Transport", "Essentials", "Health", "EMI", "Savings", "Other", "Rent", "Entertainment", "Utilities", "Investment", "Insurance"];
const typeMap = ["Bank Account", "Cash Wallet", "Credit Card", "Debit Card"];

let globalAccounts = [];
let chartWealth = null;
let chartCategory = null;

// Routing logic
function switchView(viewId) {
    document.querySelectorAll('.dashboard-content').forEach(el => el.style.display = 'none');
    document.getElementById(`view-${viewId}`).style.display = 'block';
    
    // Update sidebar
    document.querySelectorAll('.nav-links li').forEach(el => el.classList.remove('active'));
    event.currentTarget.classList.add('active');

    if(viewId === 'dashboard') { loadDashboard(); loadNews(); loadAlerts(); }
    if(viewId === 'analytics') loadAnalytics();
    if(viewId === 'cards') renderCardDropdown();
    if(viewId === 'budgets') loadBudgets();
    if(viewId === 'history') loadHistory();
}

// ... Authentication scripts from earlier ...
function switchTab(tab) {
    document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
    document.querySelectorAll('.auth-form').forEach(f => f.classList.remove('active'));
    if(tab === 'login') { document.querySelectorAll('.tab')[0].classList.add('active'); document.getElementById('login-form').classList.add('active'); }
    else { document.querySelectorAll('.tab')[1].classList.add('active'); document.getElementById('register-form').classList.add('active'); }
}

async function handleLogin(e) {
    e.preventDefault();
    const u = document.getElementById('login-username').value;
    const p = document.getElementById('login-password').value;
    try {
        const res = await fetch(`${API_BASE}/login`, { method: 'POST', body: JSON.stringify({username: u, password: p}) });
        const data = await res.json();
        if(data.success) { localStorage.setItem("nexus_user_id", data.userId); window.location.href = "dashboard.html"; }
        else alert(data.error);
    } catch(err) { alert("Server error."); }
}

async function handleRegister(e) {
    e.preventDefault();
    const f = document.getElementById('reg-name').value;
    const m = document.getElementById('reg-email').value;
    const u = document.getElementById('reg-username').value;
    const p = document.getElementById('reg-password').value;
    try {
        const res = await fetch(`${API_BASE}/register`, { method: 'POST', body: JSON.stringify({username: u, password: p, email: m, fullName: f}) });
        const data = await res.json();
        if(data.success) { localStorage.setItem("nexus_user_id", data.userId); window.location.href = "dashboard.html"; }
        else alert(data.error);
    } catch(err) { alert("Server error."); }
}

function logout() { localStorage.removeItem("nexus_user_id"); window.location.href = "index.html"; }

// Modals
function closeModal(id) { document.getElementById(id).style.display = 'none'; }
function openTransactionModal() { document.getElementById('tx-modal').style.display = 'flex'; }
function openAccountModal() { document.getElementById('acc-modal').style.display = 'flex'; }
function openCardModal() { document.getElementById('card-modal').style.display = 'flex'; }

// Dashboard
async function loadDashboard() {
    const userId = localStorage.getItem("nexus_user_id");
    if(!userId) return;

    try {
        const res = await fetch(`${API_BASE}/dashboard/${userId}`);
        const data = await res.json();
        
        if(data.success) {
            globalAccounts = data.accounts;
            document.getElementById('total-balance').innerText = `$${data.totalBalance.toFixed(2)}`;
            
            // Artificial Notification for investing
            if(data.totalBalance > 5000) {
                const notif = document.getElementById('notification-panel');
                notif.style.display = 'block';
                notif.innerHTML = `<i class="fa fa-info-circle"></i> Insight: You have $${data.totalBalance.toFixed(2)} in total wealth. Setting a Savings/Investment budget for this month could significantly mature your portfolio!`;
            }

            renderAccounts(data.accounts);
        }
    } catch(err) { console.error(err); }
}

function renderAccounts(accounts) {
    const list = document.getElementById('accounts-list');
    const select = document.getElementById('tx-account');
    list.innerHTML = '';
    
    // Preserve custom options in dropdown
    const customOpts = `
        <option value="">Select Account / Card</option>
        <option disabled>──────────</option>
        <option value="new_bank">+ Add New Bank Account</option>
        <option value="new_card">+ Add New Credit/Debit Card</option>
        <option disabled>──────────</option>
    `;
    select.innerHTML = customOpts;

    if(accounts.length === 0) return;

    accounts.forEach(acc => {
        // List Card
        const div = document.createElement('div');
        div.className = 'acc-item';
        div.onclick = () => { loadTransactions(acc.id); };
        div.innerHTML = `
            <div class="acc-info" style="cursor:pointer;" onclick="loadTransactions('${acc.id}')">
                <h4>${acc.accountName}</h4>
                <p>${typeMap[acc.type]}</p>
            </div>
            <div style="display: flex; align-items: center; justify-content: space-between;">
                <div class="acc-bal" style="cursor:pointer;" onclick="loadTransactions('${acc.id}')">$${acc.balance.toFixed(2)}</div>
                <div style="display: flex; gap: 0.5rem; margin-left: 1rem;">
                    <button class="btn-text" style="padding: 2px" onclick="openEditAccountModal('${acc.id}', '${acc.accountName}')" title="Edit"><i class="fa fa-pen"></i></button>
                    <button class="btn-text" style="padding: 2px; color: var(--danger)" onclick="handleDeleteAccount('${acc.id}')" title="Delete"><i class="fa fa-trash"></i></button>
                </div>
            </div>
        `;
        list.appendChild(div);

        // Select Option
        const opt = document.createElement('option');
        opt.value = acc.id;
        opt.innerText = `${acc.accountName} (${typeMap[acc.type]}) - $${acc.balance.toFixed(2)}`;
        select.appendChild(opt);
    });
}

document.getElementById('tx-account').addEventListener('change', function() {
    if(this.value === 'new_bank') { closeModal('tx-modal'); openAccountModal(); this.value=''; }
    if(this.value === 'new_card') { closeModal('tx-modal'); openCardModal(); this.value=''; }
});

async function handleAccountSubmit(e) {
    e.preventDefault();
    const userId = localStorage.getItem("nexus_user_id");
    try {
        const res = await fetch(`${API_BASE}/account`, {
            method: 'POST',
            body: JSON.stringify({
                userId, 
                name: document.getElementById('acc-name').value, 
                type: parseInt(document.getElementById('acc-type').value), 
                balance: parseFloat(document.getElementById('acc-bal').value)
            })
        });
        const data = await res.json();
        if(data.success) { closeModal('acc-modal'); loadDashboard(); }
    } catch(err) {}
}

async function handleTransactionSubmit(e) {
    e.preventDefault();
    try {
        const res = await fetch(`${API_BASE}/transaction`, {
            method: 'POST',
            body: JSON.stringify({
                accountId: document.getElementById('tx-account').value,
                amount: parseFloat(document.getElementById('tx-amount').value),
                type: parseInt(document.getElementById('tx-type').value),
                category: parseInt(document.getElementById('tx-category').value),
                description: document.getElementById('tx-desc').value
            })
        });
        const data = await res.json();
        if(data.success) { closeModal('tx-modal'); loadDashboard(); loadTransactions(document.getElementById('tx-account').value); }
        else alert(data.error);
    } catch(err) {}
}

async function loadTransactions(accId) {
    const list = document.getElementById('transactions-list');
    list.innerHTML = '<div class="loading">Loading...</div>';
    try {
        const res = await fetch(`${API_BASE}/transactions/${accId}`);
        const data = await res.json();
        list.innerHTML = '';
        if(data.transactions.length === 0) list.innerHTML = '<p>No recent transactions.</p>';
        data.transactions.reverse().forEach(tx => {
            const isInc = tx.type === 0;
            list.innerHTML += `
                <div class="tx-item">
                    <div>
                        <div class="desc">${tx.description}</div>
                        <div class="cat"><i class="fa fa-tag"></i> Spent On: ${catMap[tx.category]}</div>
                    </div>
                    <div class="amt ${isInc ? 'inc' : 'exp'}">${isInc ? '+' : '-'}$${tx.amount.toFixed(2)}</div>
                </div>
            `;
        });
    } catch(err) {}
}

// Analytics 
async function loadAnalytics() {
    const userId = localStorage.getItem("nexus_user_id");
    try {
        const res = await fetch(`${API_BASE}/analytics/${userId}`);
        const data = await res.json();
        if(!data.success) return;

        // Wealth Donut
        const accNames = data.accountsData.map(a => a.name);
        const accBals = data.accountsData.map(a => a.balance);
        
        if(chartWealth) chartWealth.destroy();
        chartWealth = new Chart(document.getElementById('wealthChart'), {
            type: 'doughnut',
            data: { labels: accNames, datasets: [{ data: accBals, backgroundColor: ['#3b82f6', '#10b981', '#ef4444', '#8b5cf6'] }] },
            options: { responsive: true, maintainAspectRatio: false, plugins: { legend: { position: 'bottom', labels: {color: '#f8fafc'} } } }
        });

        // Category Bar
        const catLabels = Object.keys(data.categoryData).map(k => catMap[parseInt(k)]);
        const catVals = Object.values(data.categoryData);

        if(chartCategory) chartCategory.destroy();
        chartCategory = new Chart(document.getElementById('categoryChart'), {
            type: 'bar',
            data: { labels: catLabels, datasets: [{ label: 'Total Spent', data: catVals, backgroundColor: '#c084fc' }] },
            options: { responsive: true, maintainAspectRatio: false, scales: { y: { beginAtZero: true, grid: {color: 'rgba(255,255,255,0.1)'}, ticks: {color:'#94a3b8'} }, x: { grid: {color: 'transparent'}, ticks: {color:'#94a3b8'} } }, plugins: { legend: { display: false } } }
        });

    } catch(err) { console.error(err); }
}

// Virtual Cards
function renderCardDropdown() {
    const select = document.getElementById('card-context-account');
    const newSelect = document.getElementById('new-card-account');
    select.innerHTML = '<option value="">Select Account</option>';
    newSelect.innerHTML = '<option value="">Link Card to Account</option>';
    globalAccounts.forEach(a => {
        select.innerHTML += `<option value="${a.id}">${a.accountName}</option>`;
        newSelect.innerHTML += `<option value="${a.id}">${a.accountName}</option>`;
    });
}

async function handleCardSubmit(e) {
    e.preventDefault();
    try {
        const res = await fetch(`${API_BASE}/card`, {
            method: 'POST',
            body: JSON.stringify({
                accountId: document.getElementById('new-card-account').value,
                holderName: document.getElementById('card-holder').value,
                limit: parseFloat(document.getElementById('card-limit').value)
            })
        });
        const data = await res.json();
        if(data.success) {
            closeModal('card-modal');
            document.getElementById('card-context-account').value = document.getElementById('new-card-account').value;
            loadCardsForAccount(document.getElementById('new-card-account').value);
        }
    } catch(err) {}
}

async function loadCardsForAccount(accId) {
    if(!accId) return;
    const list = document.getElementById('cards-list');
    list.innerHTML = '<div class="loading">Loading cards...</div>';
    try {
        const res = await fetch(`${API_BASE}/cards/${accId}`);
        const data = await res.json();
        list.innerHTML = '';
        if(data.cards.length === 0) { list.innerHTML = '<p>No virtual cards active.</p>'; return; }
        
        data.cards.forEach(c => {
            list.innerHTML += `
                <div class="virtual-card">
                    <div style="font-weight:600">Virtual ${c.dailyLimit > 5000 ? 'Credit' : 'Debit'}</div>
                    <div class="vcard-number">${c.cardNumber}</div>
                    <div class="vcard-details">
                        <span>${c.cardHolderName}</span>
                        <span>Exp ${c.expiryDate} &nbsp;&nbsp; CVV ***</span>
                    </div>
                </div>
            `;
        });
    } catch(err) {}
}

// User Profile
async function openProfileModal() {
    document.getElementById('profile-modal').style.display = 'flex';
    const userId = localStorage.getItem("nexus_user_id");
    try {
        const res = await fetch(`${API_BASE}/user/${userId}`);
        const data = await res.json();
        if(data.success) {
            document.getElementById('profile-details').innerHTML = `
                <p><strong>Name:</strong> ${data.user.fullName}</p>
                <p><strong>Username:</strong> @${data.user.username}</p>
                <p><strong>Email:</strong> ${data.user.email}</p>
                <p><strong>Status:</strong> ${data.user.isPremium ? '<span style="color:var(--accent)">Premium Member</span>' : 'Standard Tier'}</p>
                <p><strong>Member Since:</strong> ${new Date(data.user.createdAt * 1000).toDateString()}</p>
            `;
        }
    } catch(err) {}
}

function toggleAlerts() {
    document.getElementById('alerts-dropdown').classList.toggle('show');
}

// --- Alerts ---
async function loadAlerts() {
    const userId = localStorage.getItem("nexus_user_id");
    if(!userId) return;
    try {
        const res = await fetch(`${API_BASE}/alerts/${userId}`);
        const data = await res.json();
        if(data.success) {
            const unread = data.alerts.filter(a => !a.isRead);
            const badge = document.getElementById('alert-badge');
            if(unread.length > 0) { badge.style.display = 'block'; badge.innerText = unread.length; }
            else { badge.style.display = 'none'; }
            
            const drop = document.getElementById('alerts-dropdown');
            drop.innerHTML = '';
            if(data.alerts.length === 0) drop.innerHTML = '<div class="alert-item">No new alerts</div>';
            
            const typeMapStr = {0: 'DUE', 1: 'REMINDER', 2: 'NEWS', 3: 'WARNING'};
            data.alerts.forEach(a => {
                drop.innerHTML += `<div class="alert-item ${typeMapStr[a.type]}">${a.message}</div>`;
            });
        }
    } catch(err) {}
}

// --- News ---
async function loadNews() {
    try {
        const res = await fetch(`${API_BASE}/news`);
        const data = await res.json();
        if(data.success) {
            const list = document.getElementById('news-list');
            list.innerHTML = '';
            data.news.forEach(n => {
                list.innerHTML += `
                    <div class="news-item ${n.type}">
                        <h4>${n.title}</h4>
                        <p>${n.description}</p>
                    </div>
                `;
            });
        }
    } catch(err) {}
}

// --- Budgets ---
async function allocateBudgets() {
    const userId = localStorage.getItem("nexus_user_id");
    const salary = parseFloat(document.getElementById('budget-salary').value);
    if(!salary || salary <= 0) { alert('Please enter a valid salary'); return; }
    try {
        const res = await fetch(`${API_BASE}/budget/allocate`, {
            method: 'POST',
            body: JSON.stringify({ userId: userId, monthlySalary: salary })
        });
        const data = await res.json();
        if(data.success) loadBudgets();
    } catch(err) {}
}

async function loadBudgets() {
    const userId = localStorage.getItem("nexus_user_id");
    try {
        const res = await fetch(`${API_BASE}/budget/analysis/${userId}`);
        const data = await res.json();
        if(data.success) {
            const container = document.getElementById('budget-cards-container');
            container.innerHTML = '';
            if(data.budgets.length === 0) {
                container.innerHTML = '<p>No budgets allocated yet. Enter your custom limits or suggest a plan above.</p>';
                return;
            }
            data.budgets.sort((a,b) => b.limitAmount - a.limitAmount).forEach(b => {
                const percent = b.limitAmount > 0 ? (b.currentSpent / b.limitAmount) * 100 : 0;
                let colorClass = '';
                if(percent > 80) colorClass = 'warning';
                if(percent > 100) colorClass = 'danger';
                
                const bStr = JSON.stringify(b).replace(/"/g, '&quot;');
                container.innerHTML += `
                    <div class="budget-card" style="position: relative;">
                        <div class="budget-header">
                            <div>
                                <strong style="display: block;">${b.name || catMap[b.category]}</strong>
                                <span style="font-size: 0.75rem; color: var(--text-secondary);">${catMap[b.category]}</span>
                            </div>
                            <span class="budget-limit">$${b.currentSpent.toFixed(0)} / $${b.limitAmount.toFixed(0)}</span>
                        </div>
                        <div class="budget-progress-bg">
                            <div class="budget-progress-fill ${colorClass}" style="width: ${Math.min(percent, 100)}%;"></div>
                        </div>
                        <div style="display: flex; justify-content: flex-end; gap: 0.5rem; margin-top: 0.5rem;">
                            <button class="btn-text" style="font-size: 0.8rem;" onclick="openEditBudgetModal('${bStr}')"><i class="fa fa-pen"></i></button>
                            <button class="btn-text" style="font-size: 0.8rem; color: var(--danger);" onclick="handleDeleteBudget('${b.id}')"><i class="fa fa-trash"></i></button>
                        </div>
                    </div>
                `;
            });
        }
    } catch(err) {}
}

function updateTransactionCategories(selectId, typeValue) {
    const sel = document.getElementById(selectId);
    sel.innerHTML = '<option value="">Select Category</option>';
    if (typeValue === "1") { // Expense
        for(let i = 0; i <= 11; i++) {
            sel.innerHTML += `<option value="${i}">${catMap[i]}</option>`;
        }
    } else if (typeValue === "0") { // Income
        sel.innerHTML += `<option value="12">Salary</option>`;
        sel.innerHTML += `<option value="13">Bonus</option>`;
        sel.innerHTML += `<option value="6">Other</option>`;
    }
}

async function autoComputeBudget() {
    const salary = parseFloat(document.getElementById('budget-salary').value);
    if (!salary || salary <= 0) { alert('Enter valid salary first to suggest a plan.'); return; }
    document.getElementById('b-essentials').value = (salary * 0.30).toFixed(2);
    document.getElementById('b-emi').value = (salary * 0.20).toFixed(2);
    document.getElementById('b-savings').value = (salary * 0.20).toFixed(2);
    document.getElementById('b-investments').value = (salary * 0.15).toFixed(2);
    document.getElementById('b-insurance').value = (salary * 0.05).toFixed(2);
    document.getElementById('b-other').value = (salary * 0.10).toFixed(2);
}

async function handleAddCustomBudget(e) {
    e.preventDefault();
    const userId = localStorage.getItem("nexus_user_id");
    const category = parseInt(document.getElementById('custom-budget-category').value);
    const name = document.getElementById('custom-budget-name').value;
    const limit = parseFloat(document.getElementById('custom-budget-limit').value);
    
    const items = [{ category, name, limit }];

    try {
        const res = await fetch(`${API_BASE}/budget/set`, {
            method: 'POST',
            body: JSON.stringify({ userId: userId, items: items })
        });
        const data = await res.json();
        if(data.success) { 
            document.getElementById('custom-budget-name').value = '';
            document.getElementById('custom-budget-limit').value = '';
            loadBudgets(); 
        }
    } catch(err) {}
}

async function handleDeleteBudget(id) {
    if(!confirm("Delete this budget constraint?")) return;
    try {
        const res = await fetch(`${API_BASE}/budget/delete/${id}`, { method: 'DELETE' });
        const data = await res.json();
        if(data.success) loadBudgets();
    } catch(err) {}
}

function openEditBudgetModal(bStr) {
    const b = JSON.parse(bStr);
    document.getElementById('edit-budget-modal').style.display = 'flex';
    document.getElementById('edit-budget-id').value = b.id;
    document.getElementById('edit-budget-name').value = b.name;
    document.getElementById('edit-budget-limit').value = b.limitAmount;
}

async function handleEditBudgetSubmit(e) {
    e.preventDefault();
    try {
        const res = await fetch(`${API_BASE}/budget/edit`, {
            method: 'PUT',
            body: JSON.stringify({
                id: document.getElementById('edit-budget-id').value,
                name: document.getElementById('edit-budget-name').value,
                limitAmount: parseFloat(document.getElementById('edit-budget-limit').value)
            })
        });
        const data = await res.json();
        if(data.success) {
            closeModal('edit-budget-modal');
            loadBudgets();
        }
    } catch(err) {}
}

function openEditAccountModal(id, currentName) {
    document.getElementById('edit-acc-modal').style.display = 'flex';
    document.getElementById('edit-acc-id').value = id;
    document.getElementById('edit-acc-name').value = currentName;
}

async function handleEditAccountSubmit(e) {
    e.preventDefault();
    try {
        const res = await fetch(`${API_BASE}/account/edit`, {
            method: 'PUT',
            body: JSON.stringify({
                id: document.getElementById('edit-acc-id').value,
                name: document.getElementById('edit-acc-name').value
            })
        });
        const data = await res.json();
        if(data.success) {
            closeModal('edit-acc-modal');
            loadDashboard(); // Reload accounts list
        }
    } catch(err) {}
}

async function handleDeleteAccount(id) {
    if(!confirm("Are you sure? This will permanently delete the account and all its transactions.")) return;
    try {
        const res = await fetch(`${API_BASE}/account/delete/${id}`, { method: 'DELETE' });
        const data = await res.json();
        if(data.success) {
            loadDashboard(); // Refresh
        } else alert("Failed to delete account");
    } catch(err) {}
}

async function handleDeleteTransaction(id) {
    if(!confirm("Permanently delete this transaction? Balance will be adjusted.")) return;
    try {
        const res = await fetch(`${API_BASE}/transaction/delete/${id}`, { method: 'DELETE' });
        const data = await res.json();
        if(data.success) {
            loadHistory();
            loadDashboard(); // Refresh current balances
        } else alert("Failed to delete transaction");
    } catch(err) {}
}

async function loadHistory() {
    const userId = localStorage.getItem("nexus_user_id");
    if(!userId) return;
    const list = document.getElementById('full-history-list');
    list.innerHTML = '<tr><td colspan="6" class="loading">Loading...</td></tr>';
    
    try {
        const res = await fetch(`${API_BASE}/transactions/all/${userId}`);
        const data = await res.json();
        list.innerHTML = '';
        if(data.transactions.length === 0) { list.innerHTML = '<tr><td colspan="6" style="padding: 1rem;">No history yet.</td></tr>'; return; }
        
        let html = '';
        data.transactions.reverse().forEach(tx => {
            const isInc = tx.type === 0;
            const txStr = JSON.stringify(tx).replace(/"/g, '&quot;');
            html += `
                <tr style="border-bottom: 1px solid var(--glass-border);">
                    <td style="padding: 1rem 0;">${tx.accountName}</td>
                    <td style="padding: 1rem 0;">${tx.description}</td>
                    <td style="padding: 1rem 0;">${catMap[tx.category]}</td>
                    <td style="padding: 1rem 0; color: ${isInc ? 'var(--accent)' : 'var(--danger)'};">${isInc ? 'Income' : 'Expense'}</td>
                    <td style="padding: 1rem 0;">$${tx.amount.toFixed(2)}</td>
                    <td style="padding: 1rem 0; display: flex; gap: 0.5rem; align-items: center;">
                        <button class="btn-text" style="font-size: 0.8rem;" onclick="openEditTxModal('${txStr}')" title="Edit"><i class="fa fa-pen"></i></button>
                        <button class="btn-text" style="font-size: 0.8rem; color: var(--danger)" onclick="handleDeleteTransaction('${tx.id}')" title="Delete"><i class="fa fa-trash"></i></button>
                    </td>
                </tr>
            `;
        });
        list.innerHTML = html;
    } catch(err) {}
}

function openEditTxModal(txDataStr) {
    const tx = JSON.parse(txDataStr);
    document.getElementById('edit-tx-modal').style.display = 'flex';
    document.getElementById('edit-tx-id').value = tx.id;
    document.getElementById('edit-tx-type').value = tx.type;
    document.getElementById('edit-tx-amount').value = tx.amount;
    document.getElementById('edit-tx-desc').value = tx.description;
    
    updateTransactionCategories('edit-tx-category', tx.type.toString());
    document.getElementById('edit-tx-category').value = tx.category;
}

async function handleEditTransactionSubmit(e) {
    e.preventDefault();
    try {
        const res = await fetch(`${API_BASE}/transaction/edit`, {
            method: 'POST',
            body: JSON.stringify({
                id: document.getElementById('edit-tx-id').value,
                amount: parseFloat(document.getElementById('edit-tx-amount').value),
                category: parseInt(document.getElementById('edit-tx-category').value),
                description: document.getElementById('edit-tx-desc').value
            })
        });
        const data = await res.json();
        if(data.success) { 
            closeModal('edit-tx-modal'); 
            loadHistory(); 
            loadDashboard(); // Refresh dash numbers
        } else alert(data.error);
    } catch(err) {}
}

window.onclick = function(event) { 
    if (event.target.classList.contains('modal')) event.target.style.display = "none"; 
    if (!event.target.closest('.alert-icon')) {
        const drop = document.getElementById('alerts-dropdown');
        if (drop && drop.classList.contains('show')) drop.classList.remove('show');
    }
}
