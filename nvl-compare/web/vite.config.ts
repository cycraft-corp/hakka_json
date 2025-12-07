import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'
import path from 'path'

export default defineConfig(({ command }) => ({
  plugins: [react()],
  // Base path: '/' for dev, '/hakka_json/nvl-compare/' for GitHub Pages production
  base: command === 'serve' ? '/' : '/hakka_json/nvl-compare/',
  resolve: {
    alias: {
      '@': path.resolve(__dirname, './src'),
    },
  },
  server: {
    port: 5173,
    host: true,
  },
  build: {
    outDir: 'dist',
    sourcemap: false,
    minify: 'esbuild',
    target: 'es2020',
  },
  publicDir: 'public',
  optimizeDeps: {
    include: ['@neo4j-nvl/base', '@neo4j-nvl/react', '@neo4j-nvl/interaction-handlers'],
  },
  worker: {
    format: 'es',
  },
}))
