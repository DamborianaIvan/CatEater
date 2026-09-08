import { StrictMode } from 'react';
import { createRoot } from 'react-dom/client';
import { AuthProvider } from './app/Auth';
import { AppRouter } from './app/router';
import './styles/global.css';
import './styles/mobile.css';

createRoot(document.getElementById('root')!).render(
  <StrictMode>
    <AuthProvider>
      <AppRouter />
    </AuthProvider>
  </StrictMode>,
);
