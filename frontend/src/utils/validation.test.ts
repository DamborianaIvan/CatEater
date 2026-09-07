import { describe, expect, it } from 'vitest';
import { isValidEmail, passwordError, scheduleError } from './validation';

describe('validaciones de CatFeeder', () => {
  it('acepta emails válidos y rechaza formatos inválidos', () => {
    expect(isValidEmail('michi@example.com')).toBe(true);
    expect(isValidEmail('michi')).toBe(false);
  });
  it('valida una contraseña y su confirmación', () => {
    expect(passwordError('123')).toContain('6');
    expect(passwordError('secreta', 'otra')).toContain('coinciden');
    expect(passwordError('secreta', 'secreta')).toBeUndefined();
  });
  it('mantiene los límites del contrato de horarios', () => {
    expect(scheduleError(24, 0, 1)).toBeDefined();
    expect(scheduleError(12, 60, 1)).toBeDefined();
    expect(scheduleError(12, 30, 6)).toBeDefined();
    expect(scheduleError(12, 30, 2)).toBeUndefined();
  });
});
