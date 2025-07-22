export class Utils{
    static formatTime(ms: number): string{
        const hours        = Math.floor(ms / 3600000);
        const minutes      = Math.floor((ms % 3600000) / 60000);
        const seconds      = Math.floor((ms % 60000) / 1000);
        const milliseconds = Math.floor((ms % 1000) / 10);

        const pad = (num: number, size: number) => String(num).padStart(size, '0');

        return `${pad(hours, 2)}:${pad(minutes, 2)}:${pad(seconds, 2)}:${pad(milliseconds, 2)}`;
    }
}