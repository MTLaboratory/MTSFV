use eframe::{egui, App};
use mtsfv_core::crc32_path;
use std::path::PathBuf;
use std::thread;

#[derive(Debug)]
struct FileEntry {
    path: PathBuf,
    state: EntryState,
}

#[derive(Debug)]
enum EntryState {
    Pending(Option<thread::JoinHandle<Result<u32, String>>>),
    Done(Result<u32, String>),
}

impl FileEntry {
    fn from_path(path: PathBuf) -> Self {
        let worker_path = path.clone();
        let handle = thread::spawn(move || crc32_path(&worker_path).map_err(|e| e.to_string()));
        Self {
            path,
            state: EntryState::Pending(Some(handle)),
        }
    }
}

struct MtsfvGui {
    entries: Vec<FileEntry>,
    status: String,
}

impl MtsfvGui {
    fn add_files(&mut self) {
        if let Some(files) = rfd::FileDialog::new()
            .set_title("Select files to verify")
            .pick_files()
        {
            for path in files {
                self.entries.push(FileEntry::from_path(path));
            }
            self.status = "Calculating...".to_string();
        }
    }

    fn clear(&mut self) {
        self.entries.clear();
        self.status = "Cleared".to_string();
    }

    fn poll_workers(&mut self) {
        for entry in &mut self.entries {
            if let EntryState::Pending(handle_opt) = &mut entry.state {
                if let Some(handle) = handle_opt {
                    if handle.is_finished() {
                        let handle = handle_opt.take().expect("checked Some");
                        let result = handle
                            .join()
                            .unwrap_or_else(|_| Err("Background worker panicked".to_string()));
                        entry.state = EntryState::Done(result);
                    }
                }
            }
        }
        if self
            .entries
            .iter()
            .all(|e| matches!(e.state, EntryState::Done(_)))
        {
            self.status = "Ready".to_string();
        }
    }
}

impl Default for MtsfvGui {
    fn default() -> Self {
        Self {
            entries: Vec::new(),
            status: "Ready".to_string(),
        }
    }
}

impl App for MtsfvGui {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        self.poll_workers();

        egui::TopBottomPanel::top("header").show(ctx, |ui| {
            ui.heading("MTSFV - QuickSFV style file verifier");
            ui.horizontal(|ui| {
                if ui.button("Add files...").clicked() {
                    self.add_files();
                }
                if ui.button("Clear").clicked() {
                    self.clear();
                }
                if ui.button("Close").clicked() {
                    ui.ctx().send_viewport_cmd(egui::ViewportCommand::Close);
                }
                ui.separator();
                ui.label(format!("Status: {}", self.status));
            });
        });

        egui::CentralPanel::default().show(ctx, |ui| {
            ui.label("Selected files");
            ui.separator();

            egui::ScrollArea::vertical().show(ui, |ui| {
                egui::Grid::new("file_grid")
                    .striped(true)
                    .num_columns(3)
                    .show(ui, |ui| {
                        ui.heading("File");
                        ui.heading("CRC32");
                        ui.heading("Status");
                        ui.end_row();

                        if self.entries.is_empty() {
                            ui.label("No files selected");
                            ui.label("");
                            ui.label("");
                            ui.end_row();
                        } else {
                            for entry in &self.entries {
                                ui.label(entry.path.display().to_string());
                                match &entry.state {
                                    EntryState::Pending(_) => {
                                        ui.monospace("--");
                                        ui.label("Calculating...");
                                    }
                                    EntryState::Done(Ok(crc)) => {
                                        ui.monospace(format!("{crc:08X}"));
                                        ui.label("OK");
                                    }
                                    EntryState::Done(Err(err)) => {
                                        ui.monospace("--");
                                        ui.label(err);
                                    }
                                };
                                ui.end_row();
                            }
                        }
                    });
            });
        });
    }
}

fn main() -> eframe::Result<()> {
    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default().with_inner_size([820.0, 520.0]),
        ..Default::default()
    };
    eframe::run_native(
        "MTSFV - File Verifier",
        options,
        Box::new(|_cc| {
            Ok(Box::new(MtsfvGui::default()))
        }),
    )
}
