// Royal Mail Click & Drop API Integration
// Documentation: https://developer.royalmail.net/api/shipping/v3

const ROYAL_MAIL_API_URL = process.env.ROYAL_MAIL_API_URL || 'https://api.royalmail.net/shipping/v3';
const CLIENT_ID = process.env.ROYAL_MAIL_CLIENT_ID;
const CLIENT_SECRET = process.env.ROYAL_MAIL_CLIENT_SECRET;

let accessToken = null;
let tokenExpiry = null;

/**
 * Authenticate with Royal Mail API (OAuth2)
 */
async function authenticate() {
  // Return cached token if still valid
  if (accessToken && tokenExpiry && Date.now() < tokenExpiry) {
    return accessToken;
  }

  try {
    const response = await fetch(`${ROYAL_MAIL_API_URL}/token`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/x-www-form-urlencoded'
      },
      body: new URLSearchParams({
        grant_type: 'client_credentials',
        client_id: CLIENT_ID,
        client_secret: CLIENT_SECRET
      })
    });

    if (!response.ok) {
      throw new Error(`Authentication failed: ${response.statusText}`);
    }

    const data = await response.json();
    accessToken = data.access_token;
    // Set expiry to 5 minutes before actual expiry for safety
    tokenExpiry = Date.now() + ((data.expires_in - 300) * 1000);

    return accessToken;
  } catch (error) {
    console.error('Royal Mail authentication error:', error);
    throw error;
  }
}

/**
 * Create shipment and generate label
 *
 * @param {Object} options Shipment options
 * @returns {Object} Shipment details with tracking number and label URL
 */
async function createShipment({
  orderId,
  serviceCode = 'CRL48', // Royal Mail 48 Tracked by default
  recipient,
  packageDetails,
  returnAddress
}) {
  const token = await authenticate();

  // Build shipment request
  const shipment = {
    orderReference: orderId,
    recipient: {
      address: {
        fullName: recipient.name,
        companyName: recipient.company || '',
        addressLine1: recipient.address,
        addressLine2: '',
        addressLine3: '',
        city: recipient.city,
        county: '',
        postcode: recipient.postcode,
        countryCode: 'GB'
      },
      phoneNumber: recipient.phone || '',
      emailAddress: recipient.email
    },
    sender: {
      tradingName: 'StationBoards',
      address: {
        fullName: returnAddress.name || 'StationBoards',
        companyName: 'StationBoards Ltd',
        addressLine1: returnAddress.address,
        addressLine2: '',
        addressLine3: '',
        city: returnAddress.city,
        county: '',
        postcode: returnAddress.postcode,
        countryCode: 'GB'
      },
      phoneNumber: returnAddress.phone,
      emailAddress: returnAddress.email
    },
    packages: [{
      weightInGrams: packageDetails.weight || 400,
      packageFormatIdentifier: 'parcel', // or 'largeLetter'
      dimensions: {
        lengthInCms: packageDetails.length || 22,
        widthInCms: packageDetails.width || 14,
        heightInCms: packageDetails.height || 4
      },
      contents: packageDetails.contents || 'Electronic Display Board'
    }],
    serviceCode: serviceCode,
    serviceOptions: {
      safePlace: false,
      sendNotifications: true,
      recordedSignedFor: serviceCode.startsWith('CRL') // Tracked services
    }
  };

  try {
    const response = await fetch(`${ROYAL_MAIL_API_URL}/shipments`, {
      method: 'POST',
      headers: {
        'Authorization': `Bearer ${token}`,
        'Content-Type': 'application/json'
      },
      body: JSON.stringify(shipment)
    });

    if (!response.ok) {
      const error = await response.json();
      throw new Error(`Shipment creation failed: ${JSON.stringify(error)}`);
    }

    const data = await response.json();

    return {
      shipmentId: data.shipmentId,
      trackingNumber: data.trackingNumber,
      carrierTrackingUrl: `https://www.royalmail.com/track-your-item#/tracking-results/${data.trackingNumber}`,
      labelUrl: data.labelUrl, // URL to download PDF label
      serviceName: getServiceName(serviceCode),
      cost: data.totalShippingCost
    };

  } catch (error) {
    console.error('Royal Mail shipment creation error:', error);
    throw error;
  }
}

/**
 * Get shipping label PDF
 */
async function getShippingLabel(shipmentId) {
  const token = await authenticate();

  try {
    const response = await fetch(`${ROYAL_MAIL_API_URL}/shipments/${shipmentId}/label`, {
      headers: {
        'Authorization': `Bearer ${token}`,
        'Accept': 'application/pdf'
      }
    });

    if (!response.ok) {
      throw new Error(`Failed to get label: ${response.statusText}`);
    }

    // Return PDF as buffer
    return await response.arrayBuffer();
  } catch (error) {
    console.error('Error getting shipping label:', error);
    throw error;
  }
}

/**
 * Create manifest (required before dropping off at Post Office)
 */
async function createManifest(shipmentIds) {
  const token = await authenticate();

  try {
    const response = await fetch(`${ROYAL_MAIL_API_URL}/manifests`, {
      method: 'POST',
      headers: {
        'Authorization': `Bearer ${token}`,
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({
        shipmentIds: shipmentIds,
        manifestDate: new Date().toISOString().split('T')[0] // YYYY-MM-DD
      })
    });

    if (!response.ok) {
      const error = await response.json();
      throw new Error(`Manifest creation failed: ${JSON.stringify(error)}`);
    }

    const data = await response.json();

    return {
      manifestId: data.manifestId,
      manifestUrl: data.manifestUrl, // PDF URL
      shipmentCount: shipmentIds.length
    };

  } catch (error) {
    console.error('Manifest creation error:', error);
    throw error;
  }
}

/**
 * Get tracking updates for shipment
 */
async function getTrackingInfo(trackingNumber) {
  const token = await authenticate();

  try {
    const response = await fetch(`${ROYAL_MAIL_API_URL}/tracking/${trackingNumber}`, {
      headers: {
        'Authorization': `Bearer ${token}`
      }
    });

    if (!response.ok) {
      throw new Error(`Tracking fetch failed: ${response.statusText}`);
    }

    const data = await response.json();

    return {
      trackingNumber: data.trackingNumber,
      status: data.status,
      statusDescription: data.statusDescription,
      estimatedDelivery: data.estimatedDeliveryDate,
      events: data.events.map(event => ({
        date: event.eventDateTime,
        description: event.eventDescription,
        location: event.locationName
      }))
    };

  } catch (error) {
    console.error('Tracking fetch error:', error);
    throw error;
  }
}

/**
 * Cancel shipment (if not yet posted)
 */
async function cancelShipment(shipmentId) {
  const token = await authenticate();

  try {
    const response = await fetch(`${ROYAL_MAIL_API_URL}/shipments/${shipmentId}`, {
      method: 'DELETE',
      headers: {
        'Authorization': `Bearer ${token}`
      }
    });

    if (!response.ok) {
      throw new Error(`Cancellation failed: ${response.statusText}`);
    }

    return { success: true };
  } catch (error) {
    console.error('Cancellation error:', error);
    throw error;
  }
}

/**
 * Get service name from code
 */
function getServiceName(serviceCode) {
  const services = {
    'CRL24': 'Royal Mail 24 Tracked',
    'CRL48': 'Royal Mail 48 Tracked',
    'SD1': 'Special Delivery Guaranteed by 1pm',
    'SD9': 'Special Delivery Guaranteed by 9am',
    'TPL': 'Tracked 48 Large Letter',
    'TPS': 'Tracked 48 Small Parcel',
    'TPN': 'Tracked 48 Medium Parcel'
  };

  return services[serviceCode] || serviceCode;
}

/**
 * Get available services with pricing
 */
function getAvailableServices() {
  return [
    {
      code: 'CRL48',
      name: 'Royal Mail 48 Tracked',
      description: '2-3 working days, tracked, £50 compensation',
      price: 4.20,
      maxWeight: 2000, // grams
      recommended: true
    },
    {
      code: 'CRL24',
      name: 'Royal Mail 24 Tracked',
      description: '1 working day, tracked, £50 compensation',
      price: 5.70,
      maxWeight: 2000
    },
    {
      code: 'SD1',
      name: 'Special Delivery Guaranteed by 1pm',
      description: 'Next day by 1pm, tracked, £500 compensation',
      price: 10.50,
      maxWeight: 2000
    },
    {
      code: 'TPL',
      name: 'Tracked 48 Large Letter',
      description: '2-3 working days, tracked, £20 compensation',
      price: 2.45,
      maxWeight: 750,
      maxDimensions: {
        length: 35.3,
        width: 25,
        height: 2.5
      }
    }
  ];
}

module.exports = {
  authenticate,
  createShipment,
  getShippingLabel,
  createManifest,
  getTrackingInfo,
  cancelShipment,
  getServiceName,
  getAvailableServices
};
