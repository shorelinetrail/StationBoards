// Admin Dashboard JavaScript
// Initialize Supabase
const SUPABASE_URL = 'YOUR_SUPABASE_URL'; // Replace with your Supabase URL
const SUPABASE_ANON_KEY = 'YOUR_SUPABASE_ANON_KEY'; // Replace with your Supabase anon key

const supabase = window.supabase.createClient(SUPABASE_URL, SUPABASE_ANON_KEY);

let currentUser = null;
let orders = [];
let boards = [];

// DOM Elements
const loginScreen = document.getElementById('loginScreen');
const adminDashboard = document.getElementById('adminDashboard');
const loginForm = document.getElementById('loginForm');
const loginError = document.getElementById('loginError');
const userEmail = document.getElementById('userEmail');
const logoutBtn = document.getElementById('logoutBtn');

// Initialize
async function init() {
  // Check if user is already logged in
  const { data: { session } } = await supabase.auth.getSession();

  if (session) {
    currentUser = session.user;
    showDashboard();
  } else {
    loginScreen.style.display = 'flex';
  }
}

// Login
loginForm.addEventListener('submit', async (e) => {
  e.preventDefault();
  loginError.textContent = '';

  const email = document.getElementById('loginEmail').value;
  const password = document.getElementById('loginPassword').value;

  try {
    const { data, error } = await supabase.auth.signInWithPassword({
      email,
      password
    });

    if (error) throw error;

    currentUser = data.user;
    showDashboard();
  } catch (error) {
    console.error('Login error:', error);
    loginError.textContent = error.message;
  }
});

// Logout
logoutBtn.addEventListener('click', async () => {
  await supabase.auth.signOut();
  location.reload();
});

// Show Dashboard
function showDashboard() {
  loginScreen.style.display = 'none';
  adminDashboard.style.display = 'block';
  userEmail.textContent = currentUser.email;

  // Load initial data
  loadOrders();
  loadBoards();

  // Set up event listeners
  setupEventListeners();
}

// Tab Navigation
function setupEventListeners() {
  // Tab switching
  document.querySelectorAll('.tab').forEach(tab => {
    tab.addEventListener('click', () => {
      const tabName = tab.dataset.tab;
      switchTab(tabName);
    });
  });

  // Order filters
  document.getElementById('statusFilter').addEventListener('change', filterOrders);
  document.getElementById('searchOrders').addEventListener('input', filterOrders);

  // Add board button
  document.getElementById('addBoardBtn').addEventListener('click', () => {
    showModal('addBoardModal');
  });

  // Add board form
  document.getElementById('addBoardForm').addEventListener('submit', handleAddBoard);

  // Modal close buttons
  document.querySelectorAll('.modal-close').forEach(btn => {
    btn.addEventListener('click', (e) => {
      const modal = e.target.closest('.modal');
      hideModal(modal.id);
    });
  });

  // Close modal on background click
  document.querySelectorAll('.modal').forEach(modal => {
    modal.addEventListener('click', (e) => {
      if (e.target === modal) {
        hideModal(modal.id);
      }
    });
  });
}

function switchTab(tabName) {
  // Update tab buttons
  document.querySelectorAll('.tab').forEach(tab => {
    tab.classList.remove('active');
    if (tab.dataset.tab === tabName) {
      tab.classList.add('active');
    }
  });

  // Update tab content
  document.querySelectorAll('.tab-content').forEach(content => {
    content.classList.remove('active');
  });
  document.getElementById(tabName + 'Tab').classList.add('active');

  // Load data if needed
  if (tabName === 'orders') {
    loadOrders();
  } else if (tabName === 'boards') {
    loadBoards();
  }
}

// Load Orders
async function loadOrders() {
  try {
    const { data, error } = await supabase
      .from('orders')
      .select(`
        *,
        boards (board_id, status),
        shipments (tracking_number, status)
      `)
      .order('created_at', { ascending: false });

    if (error) throw error;

    orders = data;
    displayOrders(orders);
  } catch (error) {
    console.error('Error loading orders:', error);
    document.getElementById('ordersList').innerHTML = '<p class="error-message">Failed to load orders</p>';
  }
}

function displayOrders(ordersToDisplay) {
  const ordersList = document.getElementById('ordersList');

  if (ordersToDisplay.length === 0) {
    ordersList.innerHTML = '<p class="loading">No orders found</p>';
    return;
  }

  ordersList.innerHTML = ordersToDisplay.map(order => `
    <div class="order-card" onclick="showOrderDetail('${order.id}')">
      <div class="order-header">
        <div class="order-number">${order.order_number}</div>
        <div class="status-badge status-${order.status}">${order.status}</div>
      </div>
      <div class="order-info">
        <div class="order-info-item">
          <strong>Customer</strong>
          ${order.customer_name}
        </div>
        <div class="order-info-item">
          <strong>Email</strong>
          ${order.customer_email}
        </div>
        <div class="order-info-item">
          <strong>Quantity</strong>
          ${order.quantity} board${order.quantity > 1 ? 's' : ''}
        </div>
        <div class="order-info-item">
          <strong>Total</strong>
          £${order.total_price}
        </div>
        <div class="order-info-item">
          <strong>Date</strong>
          ${new Date(order.created_at).toLocaleDateString()}
        </div>
        <div class="order-info-item">
          <strong>Boards</strong>
          ${order.boards ? order.boards.length + ' assigned' : 'Not assigned'}
        </div>
      </div>
    </div>
  `).join('');
}

function filterOrders() {
  const statusFilter = document.getElementById('statusFilter').value;
  const searchQuery = document.getElementById('searchOrders').value.toLowerCase();

  let filtered = orders;

  // Filter by status
  if (statusFilter) {
    filtered = filtered.filter(order => order.status === statusFilter);
  }

  // Search
  if (searchQuery) {
    filtered = filtered.filter(order => {
      return order.order_number.toLowerCase().includes(searchQuery) ||
             order.customer_name.toLowerCase().includes(searchQuery) ||
             order.customer_email.toLowerCase().includes(searchQuery);
    });
  }

  displayOrders(filtered);
}

// Show Order Detail
async function showOrderDetail(orderId) {
  try {
    // Load full order details including history
    const { data: order, error } = await supabase
      .from('orders')
      .select(`
        *,
        boards (*),
        shipments (*),
        order_history (*, user:user_id(email)),
        email_log (*)
      `)
      .eq('id', orderId)
      .single();

    if (error) throw error;

    // Build modal content
    const modalBody = document.getElementById('orderModalBody');
    document.getElementById('modalOrderNumber').textContent = `Order ${order.order_number}`;

    modalBody.innerHTML = `
      <div class="detail-section">
        <h3>Customer Information</h3>
        <div class="detail-grid">
          <div class="detail-item">
            <label>Name</label>
            <div class="value">${order.customer_name}</div>
          </div>
          <div class="detail-item">
            <label>Email</label>
            <div class="value">${order.customer_email}</div>
          </div>
          <div class="detail-item">
            <label>Phone</label>
            <div class="value">${order.customer_phone || 'N/A'}</div>
          </div>
        </div>
      </div>

      <div class="detail-section">
        <h3>Shipping Address</h3>
        <div class="detail-item">
          <div class="value">
            ${order.shipping_address}<br>
            ${order.shipping_city}, ${order.shipping_postcode}<br>
            ${order.shipping_country}
          </div>
        </div>
      </div>

      <div class="detail-section">
        <h3>Order Details</h3>
        <div class="detail-grid">
          <div class="detail-item">
            <label>Quantity</label>
            <div class="value">${order.quantity} board${order.quantity > 1 ? 's' : ''}</div>
          </div>
          <div class="detail-item">
            <label>Total Price</label>
            <div class="value">£${order.total_price}</div>
          </div>
          <div class="detail-item">
            <label>Status</label>
            <div class="value">
              <select id="orderStatus" onchange="updateOrderStatus('${order.id}', this.value)">
                <option value="pending" ${order.status === 'pending' ? 'selected' : ''}>Pending</option>
                <option value="paid" ${order.status === 'paid' ? 'selected' : ''}>Paid</option>
                <option value="processing" ${order.status === 'processing' ? 'selected' : ''}>Processing</option>
                <option value="shipped" ${order.status === 'shipped' ? 'selected' : ''}>Shipped</option>
                <option value="delivered" ${order.status === 'delivered' ? 'selected' : ''}>Delivered</option>
                <option value="cancelled" ${order.status === 'cancelled' ? 'selected' : ''}>Cancelled</option>
              </select>
            </div>
          </div>
          <div class="detail-item">
            <label>Payment Status</label>
            <div class="value">
              <select id="paymentStatus" onchange="updatePaymentStatus('${order.id}', this.value)">
                <option value="pending" ${order.payment_status === 'pending' ? 'selected' : ''}>Pending</option>
                <option value="paid" ${order.payment_status === 'paid' ? 'selected' : ''}>Paid</option>
                <option value="refunded" ${order.payment_status === 'refunded' ? 'selected' : ''}>Refunded</option>
              </select>
            </div>
          </div>
        </div>
        ${order.notes ? `
          <div class="detail-item" style="margin-top: 1rem;">
            <label>Notes</label>
            <div class="value">${order.notes}</div>
          </div>
        ` : ''}
      </div>

      <div class="detail-section">
        <h3>Assigned Boards</h3>
        ${order.boards && order.boards.length > 0 ? `
          <div class="boards-table">
            <table>
              <thead>
                <tr>
                  <th>Board ID</th>
                  <th>Status</th>
                  <th>Assigned</th>
                </tr>
              </thead>
              <tbody>
                ${order.boards.map(board => `
                  <tr>
                    <td class="board-id">${board.board_id}</td>
                    <td><span class="status-badge status-${board.status}">${board.status}</span></td>
                    <td>${new Date(board.assigned_at).toLocaleString()}</td>
                  </tr>
                `).join('')}
              </tbody>
            </table>
          </div>
        ` : '<p>No boards assigned yet</p>'}
        <div class="action-buttons">
          <button class="btn-primary" onclick="showAssignBoardDialog('${order.id}')">Assign Board</button>
        </div>
      </div>

      ${order.shipments ? `
        <div class="detail-section">
          <h3>Shipping Information</h3>
          <div class="detail-grid">
            <div class="detail-item">
              <label>Tracking Number</label>
              <div class="value">${order.shipments.tracking_number || 'Not yet shipped'}</div>
            </div>
            <div class="detail-item">
              <label>Service</label>
              <div class="value">${order.shipments.service_name || 'N/A'}</div>
            </div>
            <div class="detail-item">
              <label>Status</label>
              <div class="value">${order.shipments.status || 'N/A'}</div>
            </div>
          </div>
          <div class="action-buttons">
            <button class="btn-primary" onclick="createShippingLabel('${order.id}')">Create Shipping Label</button>
          </div>
        </div>
      ` : ''}

      <div class="detail-section">
        <h3>Actions</h3>
        <div class="action-buttons">
          <button class="btn-success" onclick="sendEmail('${order.id}', 'payment_reminder')">Send Payment Reminder</button>
          <button class="btn-success" onclick="sendEmail('${order.id}', 'shipping_notification')">Send Shipping Notification</button>
          <button class="btn-warning" onclick="downloadInvoice('${order.id}')">Download Invoice</button>
        </div>
      </div>

      <div class="detail-section">
        <h3>Activity Log</h3>
        <div class="activity-log">
          ${order.order_history && order.order_history.length > 0 ?
            order.order_history.map(activity => `
              <div class="activity-item">
                <div class="timestamp">${new Date(activity.created_at).toLocaleString()}</div>
                <div class="action">${activity.action}</div>
                <div class="description">${activity.description || ''}</div>
              </div>
            `).join('') :
            '<p>No activity yet</p>'
          }
        </div>
      </div>
    `;

    showModal('orderModal');
  } catch (error) {
    console.error('Error loading order details:', error);
    alert('Failed to load order details');
  }
}

// Update Order Status
async function updateOrderStatus(orderId, newStatus) {
  try {
    const { error } = await supabase
      .from('orders')
      .update({ status: newStatus })
      .eq('id', orderId);

    if (error) throw error;

    // Log activity
    await supabase
      .from('order_history')
      .insert({
        order_id: orderId,
        action: 'status_changed',
        description: `Status changed to ${newStatus}`,
        performed_by: currentUser.email
      });

    alert('Status updated successfully');
    loadOrders(); // Refresh orders list
  } catch (error) {
    console.error('Error updating status:', error);
    alert('Failed to update status');
  }
}

// Update Payment Status
async function updatePaymentStatus(orderId, newStatus) {
  try {
    const updates = { payment_status: newStatus };
    if (newStatus === 'paid') {
      updates.paid_at = new Date().toISOString();
    }

    const { error } = await supabase
      .from('orders')
      .update(updates)
      .eq('id', orderId);

    if (error) throw error;

    // Log activity
    await supabase
      .from('order_history')
      .insert({
        order_id: orderId,
        action: 'payment_status_changed',
        description: `Payment status changed to ${newStatus}`,
        performed_by: currentUser.email
      });

    alert('Payment status updated successfully');
    loadOrders();
  } catch (error) {
    console.error('Error updating payment status:', error);
    alert('Failed to update payment status');
  }
}

// Load Boards
async function loadBoards() {
  try {
    const { data, error } = await supabase
      .from('boards')
      .select('*, orders(order_number, customer_name)')
      .order('created_at', { ascending: false });

    if (error) throw error;

    boards = data;

    // Update stats
    const stats = {
      total: boards.length,
      in_stock: boards.filter(b => b.status === 'in_stock').length,
      assigned: boards.filter(b => b.status === 'assigned').length,
      active: boards.filter(b => b.status === 'active').length
    };

    document.getElementById('totalBoards').textContent = stats.total;
    document.getElementById('inStockBoards').textContent = stats.in_stock;
    document.getElementById('assignedBoards').textContent = stats.assigned;
    document.getElementById('activeBoards').textContent = stats.active;

    // Display boards table
    displayBoardsTable(boards);
  } catch (error) {
    console.error('Error loading boards:', error);
  }
}

function displayBoardsTable(boardsToDisplay) {
  const boardsTable = document.getElementById('boardsTable');

  if (boardsToDisplay.length === 0) {
    boardsTable.innerHTML = '<p class="loading">No boards in inventory</p>';
    return;
  }

  boardsTable.innerHTML = `
    <table>
      <thead>
        <tr>
          <th>Board ID</th>
          <th>Status</th>
          <th>Order</th>
          <th>Firmware</th>
          <th>Hardware</th>
          <th>Manufactured</th>
          <th>Actions</th>
        </tr>
      </thead>
      <tbody>
        ${boardsToDisplay.map(board => `
          <tr>
            <td class="board-id">${board.board_id}</td>
            <td><span class="status-badge status-${board.status}">${board.status}</span></td>
            <td>${board.orders ? board.orders.order_number : 'N/A'}</td>
            <td>${board.firmware_version || 'N/A'}</td>
            <td>${board.hardware_revision || 'N/A'}</td>
            <td>${board.manufactured_date ? new Date(board.manufactured_date).toLocaleDateString() : 'N/A'}</td>
            <td>
              <button class="btn-secondary" onclick="editBoard('${board.id}')">Edit</button>
            </td>
          </tr>
        `).join('')}
      </tbody>
    </table>
  `;
}

// Add Board
async function handleAddBoard(e) {
  e.preventDefault();

  const boardData = {
    board_id: document.getElementById('newBoardId').value,
    firmware_version: document.getElementById('firmwareVersion').value || null,
    hardware_revision: document.getElementById('hardwareRevision').value || null,
    manufactured_date: document.getElementById('manufacturedDate').value || null,
    notes: document.getElementById('boardNotes').value || null,
    status: 'in_stock'
  };

  try {
    const { error } = await supabase
      .from('boards')
      .insert(boardData);

    if (error) throw error;

    alert('Board added successfully');
    hideModal('addBoardModal');
    document.getElementById('addBoardForm').reset();
    loadBoards();
  } catch (error) {
    console.error('Error adding board:', error);
    alert('Failed to add board: ' + error.message);
  }
}

// Modal Functions
function showModal(modalId) {
  document.getElementById(modalId).classList.add('show');
}

function hideModal(modalId) {
  document.getElementById(modalId).classList.remove('show');
}

// Placeholder functions (to be implemented)
function showAssignBoardDialog(orderId) {
  alert('Board assignment feature coming soon');
}

function createShippingLabel(orderId) {
  alert('Royal Mail integration coming soon');
}

function sendEmail(orderId, emailType) {
  alert('Email sending feature coming soon');
}

function downloadInvoice(orderId) {
  alert('Invoice generation coming soon');
}

function editBoard(boardId) {
  alert('Board editing coming soon');
}

// Initialize on page load
init();
