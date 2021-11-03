
declare class Window
{
	protected constructor();
	public show(): void;
}

declare var exports: any;

declare namespace Engine
{
	const onLoad: (callback: (configure: EngineConfigureFunction) => any) => void;
	const log: (...args: any[]) => void;

	const window: Window;

	const start: () => boolean;
}

type EngineConfigureFunction = (config: EngineConfiguration) => Promise<void>;

type EngineConfiguration = {
	name: string;
	window?: WindowConfig;
};

type WindowConfig = {
	minWidth?: number;
	minHeight?: number;
	maxWidth?: number;
	maxHeight?: number;
	width?: number;
	height?: number;
	resizable?: boolean;
	maximized?: boolean;
	fullscreen?: boolean;
	hidden?: boolean;
};
