# Merge Conflict Analysis: web_pages.h

## Branch Information
- **Current Branch**: `claude/implement-ui-improvements-017AoZSV3u5t3N6SkHkTmsvY`
- **Target Branch**: `main` (origin/main)
- **Common Ancestor**: commit `d105123` (Merge pull request #21)
- **Conflicting File**: `firmware/include/web_pages.h`

## Summary Statistics
- **Current Branch Changes**: +909 lines, -50 lines
- **Main Branch Changes**: Significant refactoring and TFL enhancements
- **Conflict Locations**: 3 major conflict zones

---

## Conflict #1: Preset Stations Section (Lines 1094-1158)

### Current Branch Implementation
**Features Added:**
- Preset category tabs (London, Major Cities, Airports, Recent)
- 18 total preset stations across categories:
  - **London**: PAD, VIC, WAT, KGX, EUS, LST
  - **Major Cities**: MAN, BHM, EDB, GLC, LDS, LIV
  - **Airports**: GTW, SRA, LTN, HWV, BHX, MIA
  - **Recent**: Dynamically populated from localStorage
- Tab-based navigation for better organization
- Recent stations tracking with localStorage persistence

**Code Structure:**
```html
<div class="preset-category-tabs">
  <button type="button" class="preset-category-tab active" data-category="london">London</button>
  <button type="button" class="preset-category-tab" data-category="major">Major Cities</button>
  <button type="button" class="preset-category-tab" data-category="airports">Airports</button>
  <button type="button" class="preset-category-tab" data-category="recent">Recent</button>
</div>

<div class="preset-stations-container">
  <div class="preset-stations" id="presets-london">
    <!-- 6 London presets -->
  </div>
  <div class="preset-stations" id="presets-major" style="display: none;">
    <!-- 6 Major city presets -->
  </div>
  <div class="preset-stations" id="presets-airports" style="display: none;">
    <!-- 6 Airport presets -->
  </div>
  <div class="preset-stations" id="presets-recent" style="display: none;">
    <!-- Dynamically populated -->
  </div>
</div>
```

### Main Branch Implementation
**Features Added:**
- Separate preset groups for National Rail and TFL services
- Dynamic switching based on service type selection
- **National Rail Presets**: CRS codes (PAD, VIC, WAT, KGX, EUS, LST)
- **TFL Underground Presets**: NaPTAN IDs (940GZZLUPAC, 940GZZLUVIC, etc.)
- `data-name` attributes for displaying station names
- TFL line filter dropdown
- TFL platform filter dropdown

**Code Structure:**
```html
<div class="preset-stations" id="nationalRailPresets">
  <button data-station="PAD" data-name="Paddington">PAD<br><small>Paddington</small></button>
  <!-- 5 more National Rail stations -->
</div>
<div class="preset-stations" id="tflPresets" style="display:none;">
  <button data-station="940GZZLUPAC" data-name="Paddington">940GZZLUPAC<br><small>Paddington</small></button>
  <!-- 5 more TFL stations -->
</div>

<!-- TFL Line Filter (NEW) -->
<div class="form-group" id="tflLineFilterGroup" style="display:none;">
  <label for="tflLineFilter">Filter by Tube Line</label>
  <select id="tflLineFilter" name="tflLineFilter">
    <option value="">All Lines</option>
  </select>
</div>

<!-- TFL Platform Filter (NEW) -->
<div class="form-group" id="tflPlatformFilterGroup" style="display:none;">
  <label for="tflPlatformFilter">Filter by Platform</label>
  <select id="tflPlatformFilter" name="tflPlatformFilter">
    <option value="">All Platforms</option>
  </select>
</div>
```

### Conflict Assessment
**Incompatibility Level**: HIGH - Structural differences

**Issues:**
1. Different organizational paradigms:
   - Current: Geographic categories (London/Cities/Airports/Recent)
   - Main: Service type categories (National Rail/TFL)
2. Main branch adds TFL line/platform filtering not present in current
3. Main branch uses `data-name` attributes for station names
4. Current branch has 3x more presets (18 vs 6 per service)
5. Recent stations feature only in current branch

**Resolution Strategy**: HYBRID APPROACH
- Adopt main's National Rail/TFL separation (critical for functionality)
- Keep current's category tabs WITHIN each service type
- Integrate TFL line/platform filters from main
- Preserve recent stations functionality

---

## Conflict #2: autoApplySettings Function (Lines 2207-2276)

### Current Branch Implementation
**Changes:**
- Added `autoSnapshot()` call before applying settings
- Automatic settings history tracking

**Code:**
```javascript
const autoApplySettings = (stationCodeOverride) => {
  // Auto-snapshot before applying changes
  autoSnapshot();

  const formData = new URLSearchParams();
  // ... rest of function unchanged
};
```

### Main Branch Implementation
**Changes:**
- Station code validation for both National Rail and TFL
- Retrieves station code from `data-stationCode` attribute or input value
- Validates format: 3-letter CRS for National Rail, 4-12 alphanumeric for TFL
- Added TFL line filter and platform filter to form data
- Enhanced error handling with async/await
- Console logging for debugging
- More informative error messages

**Code:**
```javascript
const autoApplySettings = (stationCodeOverride) => {
  const stationInput = document.getElementById('station');
  const stationValue = stationCodeOverride || stationInput.dataset.stationCode || stationInput.value;
  const serviceType = document.getElementById('serviceType').value;
  const isTfl = serviceType === '1';

  // Validate station code before sending
  const isValidNationalRail = stationValue.length === 3 && /^[A-Z]{3}$/.test(stationValue);
  const isValidTfl = stationValue.length >= 4 && stationValue.length <= 12 && /^[A-Z0-9]+$/.test(stationValue);

  if ((isTfl && !isValidTfl) || (!isTfl && !isValidNationalRail)) {
    console.log('autoApplySettings: Skipping - invalid station code:', stationValue);
    showToast("Please select a valid station before applying settings", "warning");
    return;
  }

  const formData = new URLSearchParams();
  console.log('autoApplySettings: stationCodeOverride=', stationCodeOverride, 'station name=', stationInput.value, 'station code=', stationValue);
  formData.append('serviceType', serviceType);
  formData.append('tflApiKey', document.getElementById('tflApiKey').value);
  formData.append('tflLineFilter', document.getElementById('tflLineFilter').value);
  formData.append('tflPlatformFilter', document.getElementById('tflPlatformFilter').value);
  formData.append('station', stationValue);
  // ... additional form data with console logging

  fetch("/apply", {
    method: "POST",
    body: formData
  })
  .then(async response => {
    if (!response.ok) {
      const errorText = await response.text();
      throw new Error(errorText || `HTTP ${response.status}`);
    }
    return response;
  })
  .then(() => {
    showToast("Settings updated!", "success");
    if (ws && ws.readyState === WebSocket.OPEN) {
      ws.send(JSON.stringify({command: "getState"}));
    }
  })
  .catch(error => {
    console.error('Apply settings error:', error);
    showToast("Failed to apply settings: " + error.message, "error");
  });
};
```

### Conflict Assessment
**Incompatibility Level**: MEDIUM - Both changes needed

**Issues:**
1. Current adds snapshot functionality (1 line)
2. Main adds extensive validation and TFL support (20+ lines)
3. Both changes are non-conflicting in purpose

**Resolution Strategy**: MERGE BOTH
- Keep main's validation logic and TFL support (essential)
- Add autoSnapshot() call at the beginning (preserves new feature)
- Preserve all console logging and error handling from main

---

## Conflict #3: Helper Functions Section (Lines 2378-2686)

### Current Branch Implementation
**NEW Functions Added:**
1. `resetAllPositions()` - Reset Y-position values to defaults
2. `updateUptime()` - Device uptime display formatter
3. `getCurrentSettings()` - Capture current form state
4. `applySettings(settings)` - Restore settings from snapshot
5. `autoSnapshot()` - Auto-save before changes
6. `undoSettings()` - Restore previous settings
7. `saveSnapshot()` - Manual snapshot save
8. `viewHistory()` - View settings history

**Also includes:**
- `setInterval(updateUptime, 1000)` for live uptime
- Settings history management with localStorage
- MAX_HISTORY = 10 constant

### Main Branch Implementation
**NEW Function Added:**
1. `showTflPlatformSelector(lineId)` - Display platform dropdown for selected tube line

**Code:**
```javascript
const showTflPlatformSelector = (lineId) => {
  const platformFilter = document.getElementById('tflPlatformFilter');

  // Clear platform filter first
  platformFilter.innerHTML = '<option value="">All Platforms</option>';
  platformFilter.value = '';

  // If no line selected or no lines data, return
  if (!lineId || !window.tflLinesData) {
    return;
  }

  // Find the selected line
  const selectedLine = window.tflLinesData.find(line => line.id === lineId);

  if (!selectedLine || !selectedLine.platforms || selectedLine.platforms.length === 0) {
    console.log('No platforms found for line:', lineId);
    return;
  }

  // Populate platform dropdown
  platformFilter.innerHTML = '<option value="">All Platforms</option>' +
    selectedLine.platforms.map(platform =>
      `<option value="${escapeHtml(platform)}">${escapeHtml(platform)}</option>`
    ).join('');

  console.log('Populated', selectedLine.platforms.length, 'platforms for', selectedLine.name);
};
```

### Conflict Assessment
**Incompatibility Level**: LOW - No overlap

**Issues:**
1. Current adds 8 new functions for snapshot/undo feature
2. Main adds 1 function for TFL platform selection
3. No functional overlap or conflicts

**Resolution Strategy**: KEEP BOTH
- Preserve all 8 snapshot/undo functions from current
- Add showTflPlatformSelector from main
- Maintain proper function ordering

---

## Additional Considerations

### Missing Features in Current Branch
The main branch includes several enhancements not in current:

1. **Station Name Display** (line ~1900-2000)
   - Shows station names in input field instead of just codes
   - Uses `data-stationCode` attribute to preserve code while displaying name

2. **TFL Line Fetching Improvements**
   - More robust JSON parsing for busy stations
   - Conservative memory allocation with retry logic
   - Platform detection and filtering

3. **Form Reorganization**
   - Moved display options to match apply/auto-apply behavior
   - Better field grouping

4. **Input Maxlength Changes**
   - Station input: `maxlength="50"` (current) vs `maxlength="15"` (main)
   - Main is more restrictive but allows for NaPTAN IDs

### CSS Conflicts
Likely minimal - current branch adds new classes that don't overlap:
- `.signal-bars`, `.signal-bar`
- `.device-info-panel`, `.device-info-toggle`, `.device-info-content`
- `.uptime-counter`
- `.settings-snapshot-bar`, `.btn-snapshot`

Main branch may have CSS changes that need verification.

---

## Recommended Merge Strategy

### Phase 1: Adopt Main's Structure
1. Accept main's preset organization (National Rail vs TFL)
2. Accept main's autoApplySettings with all validation
3. Accept main's TFL line/platform filtering
4. Accept main's station name display logic

### Phase 2: Integrate Current's Features
1. Add category tabs WITHIN National Rail presets (London/Cities/Airports/Recent)
2. Optionally add category tabs for TFL presets (if desired)
3. Add all snapshot/undo functions (8 functions)
4. Add autoSnapshot() call to autoApplySettings
5. Add uptime counter and signal bars CSS/HTML
6. Add device info panel CSS/HTML

### Phase 3: Testing Priorities
1. Verify TFL line/platform filtering works
2. Verify snapshot/undo functionality
3. Verify preset category tabs work with both services
4. Verify recent stations tracking
5. Verify uptime counter and signal bars display
6. Test station name display with both CRS and NaPTAN codes

---

## Merge Difficulty Assessment

**Overall Difficulty**: MEDIUM-HIGH

**Estimated Resolution Time**: 2-3 hours

**Risk Factors**:
- Large number of changes in both branches (909 vs ~500 lines)
- Structural differences in preset organization
- Need to carefully integrate validation logic
- Must preserve all new features from both branches

**Success Criteria**:
- ✅ All TFL features working (line/platform filtering)
- ✅ All snapshot/undo features working
- ✅ Enhanced status bar visible and functional
- ✅ Preset categories functional for both service types
- ✅ Recent stations tracking operational
- ✅ No regression in existing functionality

---

## Files for Reference

- Current branch commit: `71e789a`
- Main branch commit: `6801fd2`
- Common ancestor: `d105123`
- Conflict file: `firmware/include/web_pages.h`
