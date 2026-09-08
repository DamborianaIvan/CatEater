import { Cat, LogOut, Plus, Utensils } from 'lucide-react';
import { Link, NavLink, Outlet } from 'react-router-dom';
import { useAuth } from '../app/Auth';

export function AppLayout() {
  const { signOut } = useAuth();

  return (
    <div className="app-shell">
      <header className="topbar">
        <Link to="/feeders" className="brand" aria-label="CatFeeder - Mis comederos">
          <span className="brand-mark"><Cat /></span>
          <span>CatFeeder</span>
        </Link>

        <nav className="desktop-nav" aria-label="Navegación principal">
          <NavLink to="/feeders" className="nav-link"><Utensils /><span>Mis comederos</span></NavLink>
          <Link className="button small" to="/feeders/new"><Plus /><span>Vincular</span></Link>
          <button className="text-button" onClick={signOut} aria-label="Salir"><LogOut /><span>Salir</span></button>
        </nav>

        <nav className="mobile-nav" aria-label="Acciones rápidas">
          <Link className="mobile-nav-action primary" to="/feeders/new" aria-label="Vincular comedero"><Plus /></Link>
          <button className="mobile-nav-action" onClick={signOut} aria-label="Salir"><LogOut /></button>
        </nav>
      </header>
      <main><Outlet /></main>
    </div>
  );
}
