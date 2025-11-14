FROM node:20-alpine

# Set working directory
WORKDIR /app

# Copy package files
COPY package*.json ./

# Install dependencies
RUN npm install --production

# Copy application files
COPY server.js ./
COPY public ./public

# Create directories
RUN mkdir -p /app/firmware /app/data

# Expose ports
EXPOSE 3000

# Set environment variables
ENV NODE_ENV=production
ENV PORT=3000

# Volume for persistent data
VOLUME ["/app/data", "/app/firmware"]

# Start the server
CMD ["node", "server.js"]
