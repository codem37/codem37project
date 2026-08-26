// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Shared build-time TypeScript types for WebUI Lit components.

export interface WebUIStatusResponse {
  success: boolean;
  message?: string;
}

export enum ThemeMode {
  SYSTEM = 0,
  LIGHT = 1,
  DARK = 2,
}

export interface NavTabItem {
  id: string;
  label: string;
  icon: string;
}
