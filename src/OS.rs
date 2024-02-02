use windows::Win32::{
    Foundation::{BOOL, FALSE, LPARAM, RECT, TRUE},
    Graphics::Gdi::{EnumDisplayMonitors,HDC, HMONITOR, MONITORINFO, GetMonitorInfoW}
};
use std::mem;

/// Get "handle" to each connected display
/// Each HMONITOR is a special struct that can be used with Windows APIs to get screen data
/// This also includes virtual displays though
/// 
pub fn get_screens() -> Vec<HMONITOR> {
    let mut monitors: Vec<HMONITOR> = Vec::new();

    unsafe extern "system" fn monitor_enum_proc(
        hmonitor: HMONITOR,
        _hdc: HDC,
        _lprc: *mut RECT,
        dwdata: LPARAM,
    ) -> BOOL {
        let monitors = &mut *(mem::transmute::<LPARAM, *mut Vec<HMONITOR>>(dwdata));
        monitors.push(hmonitor);
        TRUE
    }

    unsafe {
        EnumDisplayMonitors(
            None,
            None,
            Some(monitor_enum_proc),
            LPARAM(&mut monitors as *mut Vec<HMONITOR> as isize),
        );
    }

    monitors
}
