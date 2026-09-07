export interface User { id?: string; email: string; }
export interface ApiMessage { message: string; }
export interface MotorInfo { motorState: boolean; portions?: number; commandId?: string | null; startHours?: string[]; }
export interface Schedule { hour: number; minute: number; portions: number; enabled: boolean; }
export interface FeederConfiguration { revision: number; stepsPerFeed: number; schedules: Schedule[]; }
export interface Feeder { _id?: string; feederId: string; userId?: string | null; feederName: string; feederLogo?: string; feederAsign?: boolean; feederQuantity?: number; lastConection?: string; motorInfo?: MotorInfo; configuration?: FeederConfiguration; }
export interface FeedingEvent { id: string; fecha: string; hora: string; cantidad?: string; accion: string; }
export interface LoginResponse { token: string; user?: User; }
export interface PairDeviceRequest { pairingCode?: string; pairingToken?: string; }
export interface ApiErrorBody { message?: string; error?: string; }
