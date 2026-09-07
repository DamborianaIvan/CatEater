export const isValidEmail = (value: string) => /^\S+@\S+\.\S+$/.test(value);

export function passwordError(password: string, confirmation?: string): string | undefined {
  if (password.length < 6) return 'La contraseña debe tener al menos 6 caracteres.';
  if (confirmation !== undefined && password !== confirmation) return 'Las contraseñas no coinciden.';
  return undefined;
}

export function scheduleError(hour: number, minute: number, portions: number): string | undefined {
  if (!Number.isInteger(hour) || hour < 0 || hour > 23) return 'La hora debe estar entre 0 y 23.';
  if (!Number.isInteger(minute) || minute < 0 || minute > 59) return 'Los minutos deben estar entre 0 y 59.';
  if (!Number.isInteger(portions) || portions < 1 || portions > 5) return 'Las porciones deben estar entre 1 y 5.';
  return undefined;
}
