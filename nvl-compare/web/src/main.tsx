import { StrictMode } from 'react';
import { createRoot } from 'react-dom/client';
import App from './App';

// Import styles in order
import './styles/theme.css';
import './styles/animations.css';
import './styles/responsive.css';
import './styles/index.css';

createRoot(document.getElementById('root')!).render(
  <StrictMode>
    <App />
  </StrictMode>,
);
