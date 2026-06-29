// =====================================================================
//  app.js — Auto Timer System (song ngu VN/EN)
//  Chay duoc ca khi KHONG co ESP32 (du lieu mau) de xem demo.
// =====================================================================

// ---------- Tu dien dich ----------
const I18N = {
  vi: {
    connecting: "Đang kết nối...", demo: "CHẾ ĐỘ DEMO (chưa có ESP32)",
    online: "Trực tuyến", ap_mode: "Chế độ AP",
    tab_relays: "Relay", tab_wifi: "WiFi", tab_settings: "Cài đặt", tab_system: "Hệ thống",
    wifi_title: "Kết nối WiFi", wifi_ssid: "Tên WiFi (SSID)", wifi_pass: "Mật khẩu",
    wifi_save: "Lưu & kết nối", wifi_scan: "Quét WiFi", wifi_scanning: "Đang quét...", wifi_none: "Không thấy mạng nào",
    ip_title: "Cấu hình IP", ip_dhcp: "Tự động (DHCP)", ip_addr: "Địa chỉ IP", ip_gw: "Gateway",
    ip_mode: "Chế độ IP", ip_auto: "Tự động (DHCP)", ip_manual: "Thủ công (Static)",
    weather_title: "Thời tiết", weather_city: "Thành phố",
    weather_note: "Hiện tên thành phố + nhiệt độ ở footer màn hình OLED.",
    ip_sn: "Subnet mask", ip_dns: "DNS", ip_mdns: "Tên mDNS",
    ip_note: "Đổi IP xong thiết bị cần khởi động lại.",
    time_title: "Đồng bộ thời gian", time_ntp: "Máy chủ NTP",
    time_gmt: "Múi giờ", time_note: "",
    time_sync: "Đồng bộ ngay (NTP)", save: "Lưu", save_all: "💾 Lưu cấu hình",
    sys_title: "Thông tin", sys_ip: "IP", sys_wifi: "WiFi", sys_rtc: "RTC (DS1307)",
    sys_uptime: "Thời gian chạy", sys_reset: "Khôi phục mặc định",
    sys_reset_hint: "Khôi phục mặc định / xoá WiFi: dùng nút vật lý (giữ 5s = xoá WiFi, 30s = reset toàn bộ).",
    pw_title: "Đổi mật khẩu", pw_old: "Mật khẩu hiện tại", pw_new: "Mật khẩu mới",
    pw_save: "Đổi mật khẩu", logout: "Đăng xuất",
    pw_ok: "Đã đổi mật khẩu", pw_wrong: "Sai mật khẩu hiện tại", pw_empty: "Nhập mật khẩu mới",
    mode_auto: "Tự động", mode_on: "Bật", mode_off: "Tắt",
    active_level: "Mức kích", active_low: "Active LOW", active_high: "Active HIGH",
    p_everyday: "Hằng ngày", p_weekdays: "Ngày thường", p_weekends: "Cuối tuần", p_once: "Một lần",
    lbl_on: "Bật", lbl_off: "Tắt", sched_name: "Tên lịch",
    saved: "Đã lưu cấu hình", saved_demo: "DEMO: đã lưu (giả lập)",
    rebooting: "Đổi cấu hình mạng — thiết bị đang khởi động lại...",
    saved_wifi: "Đã lưu WiFi, thiết bị sẽ kết nối lại", syncing: "Đang đồng bộ...",
    err_save: "Lỗi lưu!", confirm_reset: "Khôi phục mặc định? Mất hết cấu hình.",
    st_connected: "Đã kết nối", st_ap: "AP / chưa nối", st_ok: "OK", st_err: "Lỗi",
  },
  en: {
    connecting: "Connecting...", demo: "DEMO MODE (no ESP32)",
    online: "Online", ap_mode: "AP mode",
    tab_relays: "Relays", tab_wifi: "WiFi", tab_settings: "Settings", tab_system: "System",
    wifi_scan: "Scan WiFi", wifi_scanning: "Scanning...", wifi_none: "No networks found",
    ip_title: "IP configuration", ip_dhcp: "Automatic (DHCP)", ip_addr: "IP address", ip_gw: "Gateway",
    ip_mode: "IP mode", ip_auto: "Automatic (DHCP)", ip_manual: "Manual (Static)",
    weather_title: "Weather", weather_city: "City",
    weather_note: "Shows city + temperature on the OLED footer.",
    ip_sn: "Subnet mask", ip_dns: "DNS", ip_mdns: "mDNS name",
    ip_note: "Device must restart after changing IP.",
    wifi_title: "WiFi connection", wifi_ssid: "WiFi name (SSID)", wifi_pass: "Password",
    wifi_save: "Save & connect",
    time_title: "Time sync", time_ntp: "NTP server",
    time_gmt: "Timezone", time_note: "",
    time_sync: "Sync now (NTP)", save: "Save", save_all: "💾 Save config",
    sys_title: "Information", sys_ip: "IP", sys_wifi: "WiFi", sys_rtc: "RTC (DS1307)",
    sys_uptime: "Uptime", sys_reset: "Factory reset",
    sys_reset_hint: "Factory reset / clear WiFi: use the physical button (hold 5s = clear WiFi, 30s = full reset).",
    pw_title: "Change password", pw_old: "Current password", pw_new: "New password",
    pw_save: "Change password", logout: "Log out",
    pw_ok: "Password changed", pw_wrong: "Wrong current password", pw_empty: "Enter new password",
    mode_auto: "Auto", mode_on: "On", mode_off: "Off",
    active_level: "Active level", active_low: "Active LOW", active_high: "Active HIGH",
    p_everyday: "Every day", p_weekdays: "Weekdays", p_weekends: "Weekends", p_once: "Once",
    lbl_on: "On", lbl_off: "Off", sched_name: "Schedule name",
    saved: "Config saved", saved_demo: "DEMO: saved (mock)",
    rebooting: "Network changed — device is restarting...",
    saved_wifi: "WiFi saved, device will reconnect", syncing: "Syncing...",
    err_save: "Save error!", confirm_reset: "Factory reset? All config will be lost.",
    st_connected: "Connected", st_ap: "AP / not connected", st_ok: "OK", st_err: "Error",
  },
};

// Thu trong tuan: bit trong daysMask (bit0=CN/Sun ... bit6=T7/Sat)
const DAYS = [
  { vi: "T2", en: "Mon", bit: 1 }, { vi: "T3", en: "Tue", bit: 2 },
  { vi: "T4", en: "Wed", bit: 3 }, { vi: "T5", en: "Thu", bit: 4 },
  { vi: "T6", en: "Fri", bit: 5 }, { vi: "T7", en: "Sat", bit: 6 },
  { vi: "CN", en: "Sun", bit: 0 },
];
const DOW = {
  vi: ["CN","T2","T3","T4","T5","T6","T7"],
  en: ["Sun","Mon","Tue","Wed","Thu","Fri","Sat"],
};

let LANG = "vi";
const T = k => (I18N[LANG][k] ?? k);

// ---------- Du lieu mau (che do demo) ----------
const mkSched = (n, en, sh, sm, eh, em, d) =>
  ({ name: n, enabled: en, startHour: sh, startMin: sm, stopHour: eh, stopMin: em, daysMask: d });
const MOCK_CONFIG = {
  wifi_ssid: "Nha-cua-toi", wifi_pass: "", ntpServer: "pool.ntp.org",
  gmt_offset: 25200, dst_offset: 0, lang: "vi",
  dhcp: true, ip: "", gateway: "", subnet: "255.255.255.0", dns: "8.8.8.8", mdns: "autotimer",
  relays: [
    { name: "Đèn sân vườn", activeLow: true, mode: 0, schedules: [
      mkSched("Buổi tối", true, 18, 0, 22, 30, 127),
      mkSched("Rạng sáng", false, 5, 0, 6, 0, 62),
      mkSched("Lịch 3", false, 0, 0, 0, 0, 127) ]},
    { name: "Máy bơm nước", activeLow: true, mode: 0, schedules: [
      mkSched("Tưới cây", true, 6, 0, 6, 15, 127),
      mkSched("Lịch 2", false, 0, 0, 0, 0, 127),
      mkSched("Lịch 3", false, 0, 0, 0, 0, 127) ]},
    { name: "Quạt thông gió", activeLow: true, mode: 1, schedules: [
      mkSched("Lịch 1", false, 0, 0, 0, 0, 127),
      mkSched("Lịch 2", false, 0, 0, 0, 0, 127),
      mkSched("Lịch 3", false, 0, 0, 0, 0, 127) ]},
    { name: "Ổ cắm 4", activeLow: true, mode: 2, schedules: [
      mkSched("Lịch 1", false, 0, 0, 0, 0, 127),
      mkSched("Lịch 2", false, 0, 0, 0, 0, 127),
      mkSched("Lịch 3", false, 0, 0, 0, 0, 127) ]},
  ],
};

let CONFIG = null;
let DEMO = false;
let CUR = {};               // gia tri hien tai tu /api/status (ip, gateway, subnet, city)
let cityPrefilled = false;

async function api(path, opts) {
  const r = await fetch(path, opts);
  if (r.status === 401) { location.href = "/login"; throw new Error(401); }  // chua dang nhap
  if (!r.ok) throw new Error(r.status);
  return r.json();
}

// ---------- Khoi tao ----------
async function init() {
  setupTabs();
  try {
    CONFIG = await api("/api/config");
  } catch (e) {
    DEMO = true;
    CONFIG = JSON.parse(JSON.stringify(MOCK_CONFIG));
  }
  LANG = CONFIG.lang === "en" ? "en" : "vi";
  buildTimezones();
  fillForms();
  renderRelays();
  applyLang();
  refreshStatus();
  setInterval(refreshStatus, 1000);
}

// ---------- Doi ngon ngu ----------
function setLang(l) {
  LANG = l;
  if (CONFIG) CONFIG.lang = l;
  renderRelays();   // ve lai chip/nhan theo ngon ngu
  applyLang();
  if (!DEMO) saveAll(true);   // dong bo xuong thiet bi -> OLED cung doi
}

function applyLang() {
  document.documentElement.lang = LANG;
  // Dich moi phan tu co data-i18n
  document.querySelectorAll("[data-i18n]").forEach(el => {
    const key = el.dataset.i18n;
    const txt = T(key);
    // Voi <label> chua <input> ben trong: chi doi text node dau
    if (el.tagName === "LABEL" && el.firstChild) el.firstChild.nodeValue = txt + " ";
    else el.textContent = txt;
  });
  // Nhan gio bat/tat + placeholder trong cac lich
  document.querySelectorAll(".lbl-on").forEach(e => e.textContent = T("lbl_on"));
  document.querySelectorAll(".lbl-off").forEach(e => e.textContent = T("lbl_off"));
  document.querySelectorAll(".sc-name").forEach(e => e.placeholder = T("sched_name"));
  // Nut ngon ngu
  langVi.classList.toggle("active", LANG === "vi");
  langEn.classList.toggle("active", LANG === "en");
}

// ---------- Tabs ----------
// Moi muc co URL rieng (SPA): /relay /wifi /time /ip /system
const ROUTES   = { relays: "/relay", wifi: "/wifi", settings: "/settings", system: "/system" };
const PATH2TAB = { "/relay": "relays", "/wifi": "wifi", "/settings": "settings", "/system": "system" };

function showTab(tab, push) {
  document.querySelectorAll(".tab").forEach(x => x.classList.toggle("active", x.dataset.tab === tab));
  document.querySelectorAll(".page").forEach(x => x.classList.toggle("active", x.id === tab));
  document.getElementById("saveAllBtn").style.display = tab === "relays" ? "block" : "none";
  if (push) history.pushState({ tab }, "", ROUTES[tab] || "/");
}
function setupTabs() {
  document.querySelectorAll(".tab").forEach(t => {
    t.onclick = () => showTab(t.dataset.tab, true);
  });
  window.onpopstate = () => showTab(PATH2TAB[location.pathname] || "relays", false);
  const tab0 = PATH2TAB[location.pathname] || "relays";
  showTab(tab0, false);
  // Chuan hoa URL goc "/" -> "/relay" (sau khi dang nhap)
  if (!PATH2TAB[location.pathname]) history.replaceState({ tab: tab0 }, "", ROUTES[tab0]);
}

// Danh sach mui gio (offset giay, nhan)
const TZ = [
  [-39600,"GMT-11 (Samoa)"],
  [-36000,"GMT-10 (Hawaii)"],
  [-32400,"GMT-9 (Alaska)"],
  [-28800,"GMT-8 (Mỹ - Los Angeles)"],
  [-25200,"GMT-7 (Mỹ - Denver)"],
  [-21600,"GMT-6 (Mỹ - Chicago, Mexico)"],
  [-18000,"GMT-5 (Mỹ - New York, Colombia)"],
  [-14400,"GMT-4 (Canada, Venezuela)"],
  [-12600,"GMT-3:30 (Newfoundland)"],
  [-10800,"GMT-3 (Brazil, Argentina)"],
  [-7200,"GMT-2 (Nam Đại Tây Dương)"],
  [-3600,"GMT-1 (Azores, Cabo Verde)"],
  [0,"GMT+0 (Anh, Iceland)"],
  [3600,"GMT+1 (Pháp, Đức, Trung Âu)"],
  [7200,"GMT+2 (Ai Cập, Hy Lạp, Đông Âu)"],
  [10800,"GMT+3 (Nga - Moscow, Ả Rập Xê Út)"],
  [12600,"GMT+3:30 (Iran)"],
  [14400,"GMT+4 (UAE - Dubai)"],
  [16200,"GMT+4:30 (Afghanistan)"],
  [18000,"GMT+5 (Pakistan)"],
  [19800,"GMT+5:30 (Ấn Độ, Sri Lanka)"],
  [20700,"GMT+5:45 (Nepal)"],
  [21600,"GMT+6 (Bangladesh)"],
  [23400,"GMT+6:30 (Myanmar)"],
  [25200,"GMT+7 (Việt Nam, Thái Lan)"],
  [28800,"GMT+8 (Trung Quốc, Singapore)"],
  [32400,"GMT+9 (Nhật Bản, Hàn Quốc)"],
  [34200,"GMT+9:30 (Úc - Adelaide)"],
  [36000,"GMT+10 (Úc - Sydney)"],
  [39600,"GMT+11 (Solomon)"],
  [43200,"GMT+12 (New Zealand, Fiji)"],
  [46800,"GMT+13 (Tonga)"],
  [50400,"GMT+14 (Kiribati)"],
];
function buildTimezones() {
  const sel = document.getElementById("gmtOffset");
  if (sel.options.length) return;
  TZ.forEach(([secs, label]) => {
    const o = document.createElement("option");
    o.value = secs; o.textContent = label;
    sel.appendChild(o);
  });
}

function fillForms() {
  wifiSsid.value = CONFIG.wifi_ssid || "";
  ntpServer.value = CONFIG.ntpServer || "pool.ntp.org";
  gmtOffset.value = CONFIG.gmt_offset ?? 25200;
  ipMode.value = (CONFIG.dhcp !== false) ? "1" : "0";
  ipAddr.value = CONFIG.ip || "";
  ipGw.value   = CONFIG.gateway || "";
  ipSn.value   = CONFIG.subnet || "255.255.255.0";
  ipDns.value  = CONFIG.dns || "8.8.8.8";
  ipMdns.value = CONFIG.mdns || "autotimer";
  city.value   = CONFIG.city || "";
  toggleDhcp();
}

// An/hien cac o IP tinh theo che do (1=Auto/DHCP -> an)
function toggleDhcp() {
  const manual = ipMode.value !== "1";
  document.getElementById("staticFields").style.display = manual ? "block" : "none";
  // Chuyen sang Manual ma o trong -> dien san IP/Gateway/Subnet/DNS hien tai
  if (manual) {
    if (!ipAddr.value && CUR.ip)      ipAddr.value = CUR.ip;
    if (!ipGw.value   && CUR.gateway) ipGw.value   = CUR.gateway;
    if (!ipSn.value   && CUR.subnet)  ipSn.value   = CUR.subnet;
  }
}

// ---------- Render relay ----------
function renderRelays() {
  const list = document.getElementById("relayList");
  list.innerHTML = "";
  CONFIG.relays.forEach((relay, ri) => {
    const node = document.getElementById("relayTpl").content.cloneNode(true);
    const card = node.querySelector(".relay");
    card.dataset.ri = ri;

    const nameEl = node.querySelector(".relay-name");
    nameEl.value = relay.name;
    nameEl.oninput = e => relay.name = e.target.value;

    node.querySelectorAll(".seg:not(.alvl) button").forEach(b => {
      if (+b.dataset.mode === relay.mode) b.classList.add("active");
      b.onclick = () => {
        relay.mode = +b.dataset.mode;
        card.querySelectorAll(".seg:not(.alvl) button").forEach(x => x.classList.remove("active"));
        b.classList.add("active");
        if (!DEMO) setRelayMode(ri, relay.mode);
      };
    });

    const sl = node.querySelector(".sched-list");
    relay.schedules.forEach(sc => sl.appendChild(buildSched(sc)));
    list.appendChild(node);
  });
}

// ---------- Render 1 lich ----------
function buildSched(sc) {
  const n = document.getElementById("schedTpl").content.cloneNode(true);

  const en = n.querySelector(".sc-en");
  en.checked = sc.enabled;
  en.onchange = e => sc.enabled = e.target.checked;

  const nm = n.querySelector(".sc-name");
  nm.value = sc.name;
  nm.oninput = e => sc.name = e.target.value;

  // 4 o so: gio bat / phut bat / gio tat / phut tat (dinh dang 24h)
  const bind = (sel, get, set, max) => {
    const el = n.querySelector(sel);
    el.value = get();
    el.onchange = e => { const v = clampInt(e.target.value, 0, max); set(v); e.target.value = v; };
  };
  bind(".sc-sh", () => sc.startHour, v => sc.startHour = v, 23);
  bind(".sc-sm", () => sc.startMin,  v => sc.startMin  = v, 59);
  bind(".sc-eh", () => sc.stopHour,  v => sc.stopHour  = v, 23);
  bind(".sc-em", () => sc.stopMin,   v => sc.stopMin   = v, 59);

  const daysBox = n.querySelector(".days");
  const renderChips = () => {
    daysBox.innerHTML = "";
    DAYS.forEach(d => {
      const c = document.createElement("button");
      c.className = "chip" + ((sc.daysMask & (1 << d.bit)) ? " on" : "");
      c.textContent = d[LANG];
      c.onclick = () => { sc.daysMask ^= (1 << d.bit); renderChips(); };
      daysBox.appendChild(c);
    });
  };
  renderChips();

  n.querySelectorAll(".presets button").forEach(b => {
    b.onclick = () => { sc.daysMask = +b.dataset.preset; renderChips(); };
  });
  return n;
}

// ---------- Trang thai thoi gian thuc ----------
async function refreshStatus() {
  let st;
  if (DEMO) {
    const d = new Date();
    st = {
      time: d.toLocaleTimeString("vi", { hour12: false }),
      date: `${DOW[LANG][d.getDay()]} ${pad(d.getDate())}/${pad(d.getMonth()+1)}/${d.getFullYear()}`,
      wifi: false, ip: "192.168.4.1 (demo)", rtc: true, uptime: "—",
      relays: CONFIG.relays.map(r => r.mode === 1),
    };
    document.getElementById("devStatus").textContent = T("demo");
  } else {
    try { st = await api("/api/status"); } catch (e) { return; }
    document.getElementById("devStatus").textContent =
      st.wifi ? T("online") + " — " + st.ip : T("ap_mode");
    // Luu gia tri hien tai de dien san o tab Cai dat
    CUR = { ip: st.ip, gateway: st.gateway, subnet: st.subnet, city: st.city };
    if (!cityPrefilled && !city.value && st.city) { city.value = st.city; cityPrefilled = true; }
  }
  clkTime.textContent = st.time;
  clkDate.textContent = st.date;
  sysIp.textContent = st.ip;
  sysWifi.textContent = st.wifi ? T("st_connected") : T("st_ap");
  sysRtc.textContent = st.rtc ? T("st_ok") : T("st_err");
  sysUptime.textContent = st.uptime;

  document.querySelectorAll(".relay").forEach((card, i) => {
    card.classList.toggle("on", !!st.relays[i]);
  });
}

// ---------- Luu (POST len ESP32) ----------
async function saveAll(silent) {
  if (DEMO) return silent ? null : toast(T("saved_demo"));
  try {
    const r = await api("/api/config", { method: "POST",
      headers: { "Content-Type": "application/json" }, body: JSON.stringify(CONFIG) });
    if (r && r.reboot) {                      // doi cau hinh mang -> thiet bi reboot
      toast(T("rebooting"));
      const newHost = (CONFIG.dhcp === false && CONFIG.ip) ? CONFIG.ip
                    : (CONFIG.mdns ? CONFIG.mdns + ".local" : location.hostname);
      setTimeout(() => { location.href = "http://" + newHost + "/"; }, 7000);
    } else if (!silent) toast(T("saved"));
  } catch (e) { toast(T("err_save")); }
}
async function setRelayMode(ri, mode) {
  try { await api("/api/relay", { method: "POST",
    headers: { "Content-Type": "application/json" }, body: JSON.stringify({ ch: ri, mode }) }); } catch (e) {}
}
async function saveWifi() {
  CONFIG.wifi_ssid = wifiSsid.value; CONFIG.wifi_pass = wifiPass.value;
  await saveAll(true); toast(T("saved_wifi"));
}

// Quét WiFi xung quanh, hiển thị danh sách như ApSetup
async function scanWifi() {
  const list = document.getElementById("wifiList");
  list.innerHTML = `<div class="wifi-scanning">${T("wifi_scanning")}</div>`;
  let nets;
  if (DEMO) {
    nets = [{ ssid: "EETIUM WORKSPACE", rssi: -45, enc: true },
            { ssid: "Nha-hang-xom", rssi: -70, enc: true },
            { ssid: "FreeWiFi", rssi: -82, enc: false }];
  } else {
    try { nets = await api("/api/scan"); }
    catch (e) { list.innerHTML = `<div class="wifi-scanning">${T("err_save")}</div>`; return; }
  }
  nets = (nets || []).filter(n => n.ssid);
  if (!nets.length) { list.innerHTML = `<div class="wifi-scanning">${T("wifi_none")}</div>`; return; }
  nets.sort((a, b) => b.rssi - a.rssi);

  const seen = {};
  list.innerHTML = "";
  nets.forEach(n => {
    if (seen[n.ssid]) return; seen[n.ssid] = 1;
    const bars = n.rssi > -55 ? 4 : n.rssi > -65 ? 3 : n.rssi > -75 ? 2 : 1;
    const it = document.createElement("div");
    it.className = "wifi-item";
    const left = document.createElement("span");
    left.className = "ws-ssid";
    left.textContent = (n.enc ? "🔒 " : "") + n.ssid;
    const right = document.createElement("span");
    right.className = "ws-meta";
    right.textContent = "▂▄▆█".slice(0, bars) + "  " + n.rssi + "dBm";
    it.append(left, right);
    it.onclick = () => { wifiSsid.value = n.ssid; wifiPass.focus(); };
    list.appendChild(it);
  });
}

// Luu toan bo tab Cai dat (thoi gian + IP + thoi tiet) bang 1 nut
async function saveSettings() {
  CONFIG.ntpServer  = ntpServer.value;
  CONFIG.gmt_offset = +gmtOffset.value;
  CONFIG.dhcp       = (ipMode.value === "1");
  CONFIG.ip         = ipAddr.value;
  CONFIG.gateway    = ipGw.value;
  CONFIG.subnet     = ipSn.value;
  CONFIG.dns        = ipDns.value;
  CONFIG.mdns       = ipMdns.value;
  CONFIG.city       = city.value;
  await saveAll();
}
async function savePassword() {
  const oldp = pwOld.value, newp = pwNew.value;
  if (!newp) return toast(T("pw_empty"));
  if (DEMO) return toast(T("saved_demo"));
  try {
    const r = await api("/api/password", { method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ old: oldp, new: newp }) });
    if (r.ok) {
      toast(T("pw_ok"));
      setTimeout(() => location.href = "/logout", 1000);   // dang xuat -> dang nhap lai bang mk moi
    } else toast(T("pw_wrong"));
  } catch (e) { toast(T("err_save")); }
}

// ---------- Tien ich ----------
const pad = x => String(x).padStart(2, "0");
const clampInt = (v, lo, hi) => { v = parseInt(v, 10); if (isNaN(v)) v = lo; return Math.max(lo, Math.min(hi, v)); };
function toast(msg) {
  const t = document.createElement("div");
  t.textContent = msg;
  t.style.cssText = "position:fixed;bottom:80px;left:50%;transform:translateX(-50%);background:#000c;color:#fff;padding:10px 18px;border-radius:20px;z-index:99";
  document.body.appendChild(t);
  setTimeout(() => t.remove(), 1800);
}

window.addEventListener("DOMContentLoaded", init);
