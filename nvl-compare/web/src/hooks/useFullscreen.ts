import { useState, useCallback, useEffect } from 'react';

export interface UseFullscreenOptions {
  /** Element to make fullscreen (defaults to document.documentElement) */
  element?: React.RefObject<HTMLElement>;
  /** Callback when fullscreen state changes */
  onChange?: (isFullscreen: boolean) => void;
}

export interface UseFullscreenReturn {
  /** Whether currently in fullscreen mode */
  isFullscreen: boolean;
  /** Enter fullscreen mode */
  enterFullscreen: () => Promise<void>;
  /** Exit fullscreen mode */
  exitFullscreen: () => Promise<void>;
  /** Toggle fullscreen mode */
  toggleFullscreen: () => Promise<void>;
  /** Whether fullscreen is supported */
  isSupported: boolean;
}

/**
 * Hook for managing fullscreen mode
 *
 * @example
 * ```tsx
 * const { isFullscreen, toggleFullscreen } = useFullscreen();
 *
 * return (
 *   <button onClick={toggleFullscreen}>
 *     {isFullscreen ? 'Exit' : 'Enter'} Fullscreen
 *   </button>
 * );
 * ```
 */
export function useFullscreen(options: UseFullscreenOptions = {}): UseFullscreenReturn {
  const { element, onChange } = options;
  const [isFullscreen, setIsFullscreen] = useState(false);

  // Check if fullscreen API is supported
  const isSupported = typeof document !== 'undefined' && (
    'fullscreenElement' in document ||
    'webkitFullscreenElement' in document ||
    'mozFullScreenElement' in document
  );

  // Get the target element
  const getElement = useCallback(() => {
    return element?.current ?? document.documentElement;
  }, [element]);

  // Check current fullscreen state
  const checkFullscreen = useCallback(() => {
    const fullscreenElement =
      document.fullscreenElement ??
      (document as unknown as { webkitFullscreenElement?: Element }).webkitFullscreenElement ??
      (document as unknown as { mozFullScreenElement?: Element }).mozFullScreenElement;
    return fullscreenElement !== null && fullscreenElement !== undefined;
  }, []);

  // Enter fullscreen
  const enterFullscreen = useCallback(async () => {
    if (!isSupported) return;

    const el = getElement();
    try {
      if (el.requestFullscreen) {
        await el.requestFullscreen();
      } else if ((el as unknown as { webkitRequestFullscreen?: () => Promise<void> }).webkitRequestFullscreen) {
        await (el as unknown as { webkitRequestFullscreen: () => Promise<void> }).webkitRequestFullscreen();
      } else if ((el as unknown as { mozRequestFullScreen?: () => Promise<void> }).mozRequestFullScreen) {
        await (el as unknown as { mozRequestFullScreen: () => Promise<void> }).mozRequestFullScreen();
      }
    } catch (error) {
      console.error('Failed to enter fullscreen:', error);
    }
  }, [isSupported, getElement]);

  // Exit fullscreen
  const exitFullscreen = useCallback(async () => {
    if (!isSupported) return;

    try {
      if (document.exitFullscreen) {
        await document.exitFullscreen();
      } else if ((document as unknown as { webkitExitFullscreen?: () => Promise<void> }).webkitExitFullscreen) {
        await (document as unknown as { webkitExitFullscreen: () => Promise<void> }).webkitExitFullscreen();
      } else if ((document as unknown as { mozCancelFullScreen?: () => Promise<void> }).mozCancelFullScreen) {
        await (document as unknown as { mozCancelFullScreen: () => Promise<void> }).mozCancelFullScreen();
      }
    } catch (error) {
      console.error('Failed to exit fullscreen:', error);
    }
  }, [isSupported]);

  // Toggle fullscreen
  const toggleFullscreen = useCallback(async () => {
    if (isFullscreen) {
      await exitFullscreen();
    } else {
      await enterFullscreen();
    }
  }, [isFullscreen, enterFullscreen, exitFullscreen]);

  // Listen for fullscreen changes
  useEffect(() => {
    if (!isSupported) return;

    const handleChange = () => {
      const newState = checkFullscreen();
      setIsFullscreen(newState);
      onChange?.(newState);
    };

    document.addEventListener('fullscreenchange', handleChange);
    document.addEventListener('webkitfullscreenchange', handleChange);
    document.addEventListener('mozfullscreenchange', handleChange);

    return () => {
      document.removeEventListener('fullscreenchange', handleChange);
      document.removeEventListener('webkitfullscreenchange', handleChange);
      document.removeEventListener('mozfullscreenchange', handleChange);
    };
  }, [isSupported, checkFullscreen, onChange]);

  return {
    isFullscreen,
    enterFullscreen,
    exitFullscreen,
    toggleFullscreen,
    isSupported,
  };
}

export default useFullscreen;
