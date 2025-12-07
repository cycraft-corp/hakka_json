/** @type {import('tailwindcss').Config} */
export default {
  content: [
    './index.html',
    './src/**/*.{js,ts,jsx,tsx}',
  ],
  darkMode: 'class',
  theme: {
    extend: {
      // Custom color palette for JSON implementations
      colors: {
        // Base colors
        bg: {
          primary: '#0f172a',    // Slate 900
          secondary: '#1e293b',  // Slate 800
          tertiary: '#334155',   // Slate 700
        },
        text: {
          primary: '#f8fafc',    // Slate 50
          secondary: '#94a3b8',  // Slate 400
          muted: '#64748b',      // Slate 500
        },
        border: {
          DEFAULT: '#334155',    // Slate 700
          light: '#475569',      // Slate 600
        },
        accent: {
          DEFAULT: '#10b981',    // Emerald 500 (HakkaJson)
          light: '#34d399',      // Emerald 400
          dark: '#059669',       // Emerald 600
        },

        // Implementation-specific colors
        hakka: {
          DEFAULT: '#10b981',
          light: '#34d399',
          dark: '#059669',
        },
        serde: {
          DEFAULT: '#f97316',
          light: '#fb923c',
          dark: '#ea580c',
        },
        cpython: {
          DEFAULT: '#3b82f6',
          light: '#60a5fa',
          dark: '#2563eb',
        },
        golang: {
          DEFAULT: '#06b6d4',
          light: '#22d3ee',
          dark: '#0891b2',
        },
        jansson: {
          DEFAULT: '#8b5cf6',
          light: '#a78bfa',
          dark: '#7c3aed',
        },

        // Status colors
        success: '#10b981',
        warning: '#fbbf24',
        error: '#f87171',
        info: '#60a5fa',
      },

      // Typography for presentation
      fontSize: {
        // Larger base for readability
        'base': ['1.125rem', { lineHeight: '1.75rem' }],  // 18px
        'lg': ['1.25rem', { lineHeight: '1.875rem' }],   // 20px
        'xl': ['1.5rem', { lineHeight: '2rem' }],        // 24px
        '2xl': ['1.875rem', { lineHeight: '2.25rem' }],  // 30px
        '3xl': ['2rem', { lineHeight: '2.5rem' }],       // 32px
        // Stats numbers
        'stat': ['2.5rem', { lineHeight: '1', fontWeight: '700' }],
      },

      // Font families
      fontFamily: {
        sans: ['Inter', 'system-ui', 'sans-serif'],
        mono: ['JetBrains Mono', 'Menlo', 'monospace'],
      },

      // Spacing for presentation clarity
      spacing: {
        '18': '4.5rem',
        '88': '22rem',
        '128': '32rem',
      },

      // Transitions
      transitionDuration: {
        '250': '250ms',
        '350': '350ms',
      },

      // Animations
      animation: {
        'fade-in': 'fadeIn 0.3s ease-out',
        'slide-up': 'slideUp 0.3s ease-out',
        'pulse-subtle': 'pulseSubtle 2s ease-in-out infinite',
        'highlight': 'highlight 0.5s ease-out',
        'spin-slow': 'spin 2s linear infinite',
      },

      keyframes: {
        fadeIn: {
          '0%': { opacity: '0' },
          '100%': { opacity: '1' },
        },
        slideUp: {
          '0%': { opacity: '0', transform: 'translateY(10px)' },
          '100%': { opacity: '1', transform: 'translateY(0)' },
        },
        pulseSubtle: {
          '0%, 100%': { opacity: '1' },
          '50%': { opacity: '0.7' },
        },
        highlight: {
          '0%': { boxShadow: '0 0 0 0 rgba(16, 185, 129, 0.4)' },
          '100%': { boxShadow: '0 0 0 8px rgba(16, 185, 129, 0)' },
        },
      },

      // Box shadows for depth
      boxShadow: {
        'glow': '0 0 20px rgba(16, 185, 129, 0.3)',
        'glow-error': '0 0 20px rgba(248, 113, 113, 0.3)',
        'panel': '0 4px 6px -1px rgba(0, 0, 0, 0.3)',
      },

      // Border radius
      borderRadius: {
        'xl': '1rem',
        '2xl': '1.5rem',
      },
    },
  },
  plugins: [],
};
