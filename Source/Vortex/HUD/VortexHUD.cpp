// Fill out your copyright notice in the Description page of Project Settings.


#include "VortexHUD.h"
#include "CharacterOverlay.h"
#include "ElimAnnouncement.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Vortex/HUD/Announcement.h"

void AVortexHUD::BeginPlay() {
	Super::BeginPlay();
	
}

void AVortexHUD::AddCharacterToOverlay() {
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && CharacterOverlayClass) {
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void AVortexHUD::AddAnnouncement() {
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && AnnouncementClass) {
		Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);
		Announcement->AddToViewport();
	}
}

void AVortexHUD::AddElimAnnouncement(FString Attacker, FString Victim) {
	OwingPlayer = OwingPlayer == nullptr ? GetOwningPlayerController() : OwingPlayer;
	if (OwingPlayer && ElimAnnouncementClass) {
		UElimAnnouncement* 	ElimAnnouncementWidget = CreateWidget<UElimAnnouncement>(OwingPlayer, ElimAnnouncementClass);
		if (ElimAnnouncementWidget) {
			ElimAnnouncementWidget->SetElimAnnouncementText(Attacker, Victim);
			ElimAnnouncementWidget->AddToViewport();

			for (auto Msg: ElimMessages) {
				if (Msg && Msg->AnnouncementBox) {
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->AnnouncementBox);
					if (CanvasSlot) {
						FVector2D Position = CanvasSlot->GetPosition();
						FVector2D NewPosition(Position.X, Position.Y - CanvasSlot->GetSize().Y);
						CanvasSlot->SetPosition(NewPosition);
					}
					
				}
			}
			
			ElimMessages.Add(ElimAnnouncementWidget);

			FTimerHandle ElimMsgTimer;
			FTimerDelegate ElimMsgDelegate;
			ElimMsgDelegate.BindUFunction(this,FName("ElimAnnouncementTimeFinished"), ElimAnnouncementWidget);
			GetWorldTimerManager().SetTimer(
				ElimMsgTimer,
				ElimMsgDelegate,
				ElimAnnouncementTime,
				false);
		}
	}
}

void AVortexHUD::ElimAnnouncementTimeFinished(UElimAnnouncement* MsgToRemove) {
	if (MsgToRemove) {
		MsgToRemove->RemoveFromParent();
	}	
}

void AVortexHUD::DrawHUD() {
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine) {
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter = ViewportSize / 2;
		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;
		if (HUDPackage.CrosshairCenter) {
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairCenter, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairLeft) {
			FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairLeft, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairRight) {
			FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairRight, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairTop) {
			FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairTop, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairBottom) {
			FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairBottom, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
	}
}

void AVortexHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter,FVector2D Spread, FLinearColor CrossHairColor) {
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(ViewportCenter.X - TextureWidth / 2.f + Spread.X,
		ViewportCenter.Y - TextureHeight / 2.f + Spread.Y);
	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight,
		0.f, 0.f, 1.f, 1.f, CrossHairColor);
}
