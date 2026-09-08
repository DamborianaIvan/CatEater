import { useState, type FormEvent, type ReactNode } from 'react';
import { Cat } from 'lucide-react';
import { Link, useNavigate, useSearchParams } from 'react-router-dom';
import { authApi } from '../services/catfeeder';
import { ApiError } from '../services/api';
import { useAuth } from '../app/Auth';
import { Button, Notice } from '../components/ui';

const emailValid = (value: string) => /^\S+@\S+\.\S+$/.test(value);

function AuthShell({ title, subtitle, children }: { title: string; subtitle: string; children: ReactNode }) {
  return (
    <main className="auth-page">
      <div className="auth-container">
        <div className="auth-brand"><span className="brand-mark"><Cat /></span><b>CatFeeder</b></div>
        <section className="auth-card">
          <div className="auth-copy"><h1>{title}</h1><p>{subtitle}</p></div>
          {children}
        </section>
      </div>
    </main>
  );
}

export function LoginPage() {
  const navigate = useNavigate(); const { signIn } = useAuth();
  const [email, setEmail] = useState(''); const [password, setPassword] = useState(''); const [error, setError] = useState(''); const [loading, setLoading] = useState(false);
  async function submit(e: FormEvent) { e.preventDefault(); if (!emailValid(email) || !password) return setError('Ingresá un email válido y tu contraseña.'); setLoading(true); setError(''); try { const response = await authApi.login(email, password); signIn(response.token); navigate('/feeders', { replace: true }); } catch (err) { setError(err instanceof ApiError ? err.message : 'No pudimos iniciar sesión.'); } finally { setLoading(false); } }
  return <AuthShell title="Bienvenido de nuevo" subtitle="Administrá la alimentación de tu mascota desde un solo lugar."><form className="auth-form" onSubmit={submit}><label>Email<input type="email" autoComplete="email" value={email} onChange={(e) => setEmail(e.target.value)} placeholder="tu@email.com" /></label><label>Contraseña<input type="password" autoComplete="current-password" value={password} onChange={(e) => setPassword(e.target.value)} placeholder="Ingresá tu contraseña" /></label><Notice message={error} /><Button loading={loading} type="submit">Ingresar</Button></form><Link className="auth-link" to="/forgot-password">¿Olvidaste tu contraseña?</Link><p className="fine-print">¿Todavía no tenés cuenta? <Link to="/register">Registrarme</Link></p></AuthShell>;
}

export function RegisterPage() { const navigate = useNavigate(); const [email, setEmail] = useState(''); const [password, setPassword] = useState(''); const [confirmation, setConfirmation] = useState(''); const [error, setError] = useState(''); const [success, setSuccess] = useState(''); const [loading, setLoading] = useState(false); async function submit(e: FormEvent) { e.preventDefault(); if (!emailValid(email)) return setError('Ingresá un email válido.'); if (password.length < 6) return setError('La contraseña debe tener al menos 6 caracteres.'); if (password !== confirmation) return setError('Las contraseñas no coinciden.'); setLoading(true); setError(''); try { await authApi.register(email, password); setSuccess('Cuenta creada. Ya podés iniciar sesión.'); setTimeout(() => navigate('/login'), 900); } catch (err) { setError(err instanceof ApiError ? err.message : 'No pudimos crear tu cuenta.'); } finally { setLoading(false); } } return <AuthShell title="Creá tu cuenta" subtitle="Empezá a cuidar sus comidas, estés donde estés."><form className="auth-form" onSubmit={submit}><label>Email<input type="email" value={email} onChange={(e) => setEmail(e.target.value)} placeholder="tu@email.com" /></label><label>Contraseña<input type="password" value={password} onChange={(e) => setPassword(e.target.value)} placeholder="Creá una contraseña" /></label><label>Confirmar contraseña<input type="password" value={confirmation} onChange={(e) => setConfirmation(e.target.value)} placeholder="Repetí la contraseña" /></label><Notice message={error} /><Notice message={success} kind="success" /><Button loading={loading}>Crear cuenta</Button></form><Link className="auth-link" to="/login">Volver a iniciar sesión</Link></AuthShell>; }

export function ForgotPasswordPage() { const [email, setEmail] = useState(''); const [error, setError] = useState(''); const [success, setSuccess] = useState(''); const [loading, setLoading] = useState(false); async function submit(e: FormEvent) { e.preventDefault(); if (!emailValid(email)) return setError('Ingresá un email válido.'); setLoading(true); setError(''); try { const response = await authApi.forgot(email); setSuccess(response.message); } catch (err) { setError(err instanceof ApiError ? err.message : 'No pudimos procesar la solicitud.'); } finally { setLoading(false); } } return <AuthShell title="Recuperá tu acceso" subtitle="Te enviaremos las instrucciones a tu email."><form className="auth-form" onSubmit={submit}><label>Email<input type="email" value={email} onChange={(e) => setEmail(e.target.value)} placeholder="tu@email.com" /></label><Notice message={error} /><Notice message={success} kind="success" /><Button loading={loading}>Enviar instrucciones</Button></form><Link className="auth-link" to="/login">Volver</Link></AuthShell>; }

export function ResetPasswordPage() { const [params] = useSearchParams(); const token = params.get('token') ?? ''; const [password, setPassword] = useState(''); const [confirmation, setConfirmation] = useState(''); const [error, setError] = useState(''); const [success, setSuccess] = useState(''); const [loading, setLoading] = useState(false); async function submit(e: FormEvent) { e.preventDefault(); if (!token) return setError('El enlace de recuperación no es válido.'); if (password.length < 6) return setError('La contraseña debe tener al menos 6 caracteres.'); if (password !== confirmation) return setError('Las contraseñas no coinciden.'); setLoading(true); setError(''); try { const response = await authApi.reset(token, password); setSuccess(response.message); } catch (err) { setError(err instanceof ApiError ? err.message : 'No pudimos restablecer la contraseña.'); } finally { setLoading(false); } } return <AuthShell title="Elegí una nueva contraseña" subtitle="Usá una contraseña segura de al menos 6 caracteres."><form className="auth-form" onSubmit={submit}><label>Nueva contraseña<input type="password" value={password} onChange={(e) => setPassword(e.target.value)} placeholder="Nueva contraseña" /></label><label>Confirmar contraseña<input type="password" value={confirmation} onChange={(e) => setConfirmation(e.target.value)} placeholder="Repetí la contraseña" /></label><Notice message={error} /><Notice message={success} kind="success" /><Button loading={loading}>Restablecer contraseña</Button></form><Link className="auth-link" to="/login">Volver a iniciar sesión</Link></AuthShell>; }
