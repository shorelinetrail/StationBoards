// Smooth scrolling for anchor links
document.querySelectorAll('a[href^="#"]').forEach(anchor => {
  anchor.addEventListener('click', function (e) {
    e.preventDefault();
    const target = document.querySelector(this.getAttribute('href'));
    if (target) {
      const offset = 80; // Account for fixed navbar
      const targetPosition = target.offsetTop - offset;
      window.scrollTo({
        top: targetPosition,
        behavior: 'smooth'
      });
    }
  });
});

// Quantity selector pricing update
const quantitySelect = document.getElementById('quantity');
const subtotalEl = document.getElementById('subtotal');
const totalEl = document.getElementById('total');

const pricing = {
  1: 89,
  2: 168,
  3: 240,
  5: 385
};

if (quantitySelect) {
  quantitySelect.addEventListener('change', (e) => {
    const quantity = parseInt(e.target.value);
    const price = pricing[quantity];
    subtotalEl.textContent = `£${price}.00`;
    totalEl.textContent = `£${price}.00`;
  });
}

// Order form submission
const orderForm = document.getElementById('orderForm');
const orderSuccess = document.getElementById('orderSuccess');

if (orderForm) {
  orderForm.addEventListener('submit', async (e) => {
    e.preventDefault();

    // Disable submit button to prevent double submission
    const submitBtn = orderForm.querySelector('button[type="submit"]');
    const originalBtnText = submitBtn.textContent;
    submitBtn.disabled = true;
    submitBtn.textContent = 'Submitting...';

    // Get form data
    const formData = new FormData(orderForm);
    const data = {
      name: formData.get('name'),
      email: formData.get('email'),
      phone: formData.get('phone'),
      address: formData.get('address'),
      city: formData.get('city'),
      postcode: formData.get('postcode'),
      quantity: parseInt(formData.get('quantity')),
      notes: formData.get('notes')
    };

    try {
      // Send order to API
      const response = await fetch('/api/submit-order', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify(data)
      });

      const result = await response.json();

      if (response.ok) {
        // Show success message
        orderForm.style.display = 'none';
        orderSuccess.style.display = 'block';

        // Update success message with order number
        const successMessage = orderSuccess.querySelector('p');
        if (result.order && result.order.orderNumber) {
          successMessage.innerHTML = `Thank you for your order! Your order number is <strong>${result.order.orderNumber}</strong>. We've sent a confirmation email to ${data.email} with payment instructions.`;
        }

        console.log('Order submitted successfully:', result);
      } else {
        throw new Error(result.error || 'Order submission failed');
      }
    } catch (error) {
      console.error('Error submitting order:', error);

      // Show error message
      alert(`Failed to submit order: ${error.message}\n\nPlease try again or contact us at orders@stationboards.co.uk`);

      // Re-enable submit button
      submitBtn.disabled = false;
      submitBtn.textContent = originalBtnText;

      // Fallback: open email client with order details
      const shouldUseFallback = confirm('Would you like to send your order via email instead?');
      if (shouldUseFallback) {
        const subject = encodeURIComponent('StationBoard Order Request');
        const body = encodeURIComponent(`
Name: ${data.name}
Email: ${data.email}
Phone: ${data.phone}

Shipping Address:
${data.address}
${data.city}, ${data.postcode}

Quantity: ${data.quantity}
Total: ${totalEl.textContent}

Additional Notes:
${data.notes || 'None'}
        `);

        window.location.href = `mailto:orders@stationboards.co.uk?subject=${subject}&body=${body}`;
      }
    }
  });
}

// Reset form function
function resetForm() {
  orderForm.reset();
  orderForm.style.display = 'block';
  orderSuccess.style.display = 'none';
  subtotalEl.textContent = '£89.00';
  totalEl.textContent = '£89.00';

  // Scroll to top of form
  document.getElementById('order').scrollIntoView({ behavior: 'smooth' });
}

// Navbar background on scroll
let lastScroll = 0;
const navbar = document.querySelector('.navbar');

window.addEventListener('scroll', () => {
  const currentScroll = window.pageYOffset;

  if (currentScroll > 100) {
    navbar.style.background = 'rgba(255, 255, 255, 0.98)';
  } else {
    navbar.style.background = 'rgba(255, 255, 255, 0.95)';
  }

  lastScroll = currentScroll;
});

// Animate board preview on scroll
const boardPreview = document.querySelector('.board-screen');

const observer = new IntersectionObserver((entries) => {
  entries.forEach(entry => {
    if (entry.isIntersecting) {
      entry.target.style.animation = 'fadeInUp 0.8s ease-out';
    }
  });
}, {
  threshold: 0.1
});

if (boardPreview) {
  observer.observe(boardPreview);
}

// Add CSS animation
const style = document.createElement('style');
style.textContent = `
  @keyframes fadeInUp {
    from {
      opacity: 0;
      transform: translateY(30px) rotateY(-5deg) rotateX(2deg);
    }
    to {
      opacity: 1;
      transform: translateY(0) rotateY(-5deg) rotateX(2deg);
    }
  }
`;
document.head.appendChild(style);

// Log page view (add analytics tracking here)
console.log('StationBoards website loaded');
console.log('Ready to take orders! 🚉');
