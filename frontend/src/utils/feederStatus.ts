const ONLINE_THRESHOLD_MS = 90_000;

export function isFeederOnline(lastConnection?: string | null): boolean {
  if (!lastConnection) return false;

  const timestamp = new Date(lastConnection).getTime();
  if (Number.isNaN(timestamp)) return false;

  const elapsed = Date.now() - timestamp;
  return elapsed >= 0 && elapsed <= ONLINE_THRESHOLD_MS;
}

export function formatLastConnection(lastConnection?: string | null): string {
  if (!lastConnection) return 'Nunca conectado';

  const timestamp = new Date(lastConnection).getTime();
  if (Number.isNaN(timestamp)) return 'Fecha inválida';

  const elapsed = Math.max(0, Date.now() - timestamp);
  const seconds = Math.floor(elapsed / 1000);

  if (seconds < 60) return `hace ${seconds} ${seconds === 1 ? 'segundo' : 'segundos'}`;

  const minutes = Math.floor(seconds / 60);
  if (minutes < 60) return `hace ${minutes} ${minutes === 1 ? 'minuto' : 'minutos'}`;

  const hours = Math.floor(minutes / 60);
  if (hours < 24) return `hace ${hours} ${hours === 1 ? 'hora' : 'horas'}`;

  const days = Math.floor(hours / 24);
  return `hace ${days} ${days === 1 ? 'día' : 'días'}`;
}
