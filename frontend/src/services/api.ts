import type { ApiErrorBody } from '../types';

const baseUrl = (import.meta.env.VITE_API_URL as string | undefined)?.replace(/\/$/, '') || 'https://cat-feeder.onrender.com';
export class ApiError extends Error { constructor(public status: number, message: string) { super(message); this.name = 'ApiError'; } }
let onUnauthorized: (() => void) | undefined;
export const setUnauthorizedHandler = (handler: () => void) => { onUnauthorized = handler; };
export const getToken = () => localStorage.getItem('catfeeder.token');
async function request<T>(path: string, init: RequestInit = {}): Promise<T> {
  const token = getToken(); const headers = new Headers(init.headers); headers.set('Content-Type', 'application/json');
  if (token) headers.set('Authorization', token.startsWith('Bearer ') ? token : `Bearer ${token}`);
  let response: Response;
  try { response = await fetch(`${baseUrl}${path}`, { ...init, headers, signal: AbortSignal.timeout(20_000) }); }
  catch { throw new ApiError(0, 'No pudimos conectarnos con el servidor. Intentá nuevamente.'); }
  const body = await response.json().catch(() => ({})) as T & ApiErrorBody;
  if (!response.ok) { if (response.status === 401) onUnauthorized?.(); throw new ApiError(response.status, body.message || body.error || 'No pudimos completar la operación. Intentá nuevamente.'); }
  return body;
}
export const api = { get: <T>(path: string) => request<T>(path), post: <T>(path: string, body?: unknown) => request<T>(path, { method: 'POST', body: JSON.stringify(body) }), put: <T>(path: string, body: unknown) => request<T>(path, { method: 'PUT', body: JSON.stringify(body) }), delete: <T>(path: string) => request<T>(path, { method: 'DELETE' }) };
