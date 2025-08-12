using Microsoft.AspNetCore.Mvc;
using Microsoft.EntityFrameworkCore;
using VejrstationApi.Data;
using VejrstationApi.Dtos;

namespace VejrstationApi.Controllers;

[ApiController]
[Route("api/[controller]")]
public class MålingerController : ControllerBase
{
    private readonly VejrDbContext _db;
    public MålingerController(VejrDbContext db) => _db = db;

    // retunere data fra sidste måling
    [HttpGet("seneste")]
    public async Task<ActionResult<MålingDto>> HentSenesteEn()
    {
        // henter den nyeste måling
        var m = await _db.Målinger
            .AsNoTracking() 
            .OrderByDescending(x => x.Tidspunkt) 
            .Select(x => new MålingDto(x.Tidspunkt, x.Temperatur, x.Luftfugtighed, x.Tryk))
            .FirstOrDefaultAsync(); 

        if (m is null) return NotFound("Ingen målinger."); //hvis ingen data er fundet

        return Ok(m); //retunere seneste måling
    }

    // retunere data mellem et valgt tidsrum
    [HttpGet("interval")]
    public async Task<ActionResult<IEnumerable<MålingDto>>> HentInterval(
        [FromQuery] DateTimeOffset from, [FromQuery] DateTimeOffset to)
    {
        //validere at slittidspunktet er senere end start
        if (to <= from) return BadRequest("`to` skal være senere end `from`.");

        // Konverterer til lokale DateTime-objekter (fjerner tidszoneinfo)
        var f = from.LocalDateTime;
        var t = to.LocalDateTime;

        // Henter data mellem tidspunkterne og sorterer stigende
        var data = await _db.Målinger
            .AsNoTracking()
            .Where(m => m.Tidspunkt >= f && m.Tidspunkt <= t)
            .OrderBy(m => m.Tidspunkt)
            .Select(m => new MålingDto(m.Tidspunkt, m.Temperatur, m.Luftfugtighed, m.Tryk))
            .ToListAsync();

        return Ok(data); // Returnerer listen af målinger
    }
}
