export interface Device {
  initialize(): boolean;
  release(): boolean;
  changeDisplayState(on?: boolean): boolean;
}

export interface Display {
  writeText(
    text: string,
    x: number,
    y: number,
    color: number,
    backgroundColor: number
  ): boolean;
  drawBitmap(path: string): boolean;
  drawPng(path: string): boolean;
  drawBitmapAt(path: string, x: number, y: number): boolean;
  drawPngAt(path: string, x: number, y: number): boolean;
  setPixel(x: number, y: number, color: number): boolean;
  drawImageBuffer(buffer: number[][]): boolean;
  clear(clearColor?: number): boolean;
  clearRectangle(
    x1: number,
    y1: number,
    x2: number,
    y2: number,
    color?: number
  ): boolean;
}

export interface Keys {
  getState(): boolean[];
}

declare const ws144: {
  Device: Device;
  Display: Display;
  Keys: Keys;
};

export = ws144;
